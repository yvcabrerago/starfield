#include <SDL3/SDL.h>
#include <SDL3/SDL_mouse.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <windows.h>

#define MAX_STARS 100
#define WIDTH 640
#define HEIGHT 480
#define SCALE 1

const float mouse_threshold = 3.0f;
const float speed = .2f;

uint32_t framebuffer[WIDTH * HEIGHT];

typedef struct {
  float X;
  float Y;
  float Z;
} Star;

Star starfield[MAX_STARS];

int main(int argc, char **argv) {
  HWND preview_window = NULL;
  SDL_WindowFlags window_flags = SDL_WINDOW_FULLSCREEN | SDL_WINDOW_BORDERLESS;
  if (argc >= 3) {
    if (strcmp(argv[1], "/p") == 0) {
      preview_window = (HWND)(uintptr_t)strtoull(argv[2], NULL, 0);
      if (!IsWindow(preview_window)) {
        return EXIT_FAILURE;
      }
    }
  }

  if (argc == 2) {
    if (strncmp(argv[1], "/c", 2) == 0) {
      MessageBox(NULL, "This screen saver has no options that you can set.",
                 "Starfield Screen Saver", MB_OK);
      return EXIT_FAILURE;
    }
  }

  SDL_Window *window;
  SDL_Renderer *renderer;
  SDL_Texture *texture;
  SDL_Event event;

  if (!SDL_Init(SDL_INIT_VIDEO)) {
    return EXIT_FAILURE;
  }

  if (!SDL_CreateWindowAndRenderer("Starfield Screen Saver", WIDTH * SCALE,
                                   HEIGHT * SCALE, window_flags, &window,
                                   &renderer)) {
    SDL_Quit();
    return EXIT_FAILURE;
  }

  SDL_HideCursor();

  SDL_PropertiesID props = SDL_GetWindowProperties(window);
  HWND hwnd =
      SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WIN32_HWND_POINTER, NULL);

  if (preview_window) {
    LONG_PTR style = GetWindowLongPtr(hwnd, GWL_STYLE);
    style = (style & ~WS_POPUP) | WS_CHILD;
    SetWindowLongPtr(hwnd, GWL_STYLE, style);
    SetParent(hwnd, preview_window);
    RECT rect;
    GetClientRect(preview_window, &rect);
    SetWindowPos(hwnd, NULL, 0, 0, rect.right, rect.bottom,
                 SWP_NOZORDER | SWP_SHOWWINDOW | SWP_FRAMECHANGED);
  }

  texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_XRGB8888,
                              SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);
  if (!texture) {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return EXIT_FAILURE;
  }

  SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_LINEAR);

  for (size_t i = 0; i < MAX_STARS; i++) {
    starfield[i].X = SDL_randf() * WIDTH - WIDTH / 2.0f;
    starfield[i].Y = SDL_randf() * HEIGHT - HEIGHT / 2.0f;
    starfield[i].Z = 1.0f;
  }

  uint64_t last_time = SDL_GetTicks();
  uint8_t is_running = 1;
  int16_t star_x;
  int16_t star_y;
  while (is_running) {
    // update
    if (preview_window && !IsWindow(preview_window)) {
      is_running = 0;
    }

    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_EVENT_QUIT || event.type == SDL_EVENT_KEY_DOWN) {
        is_running = 0;
      } else if (event.type == SDL_EVENT_MOUSE_MOTION) {
        if (fabsf(event.motion.xrel) > mouse_threshold ||
            fabsf(event.motion.yrel) > mouse_threshold) {
          is_running = 0;
        }
      }
    }

    uint64_t current_time = SDL_GetTicks();
    float dt = (current_time - last_time) / 1000.0f;
    last_time = current_time;
    SDL_memset(framebuffer, 0, sizeof(framebuffer));

    for (size_t i = 0; i < MAX_STARS; i++) {
      starfield[i].Z -= dt * speed;

      if (starfield[i].Z <= 0) {
        starfield[i].Z = 1.0f;
      }

      star_x = starfield[i].X / starfield[i].Z + WIDTH / 2;
      star_y = starfield[i].Y / starfield[i].Z + HEIGHT / 2;

      if (star_x >= 0 && star_x < WIDTH && star_y >= 0 && star_y < HEIGHT) {
        framebuffer[star_x + star_y * WIDTH] = 0xFFFFFF;
      } else {
        starfield[i].X = SDL_randf() * WIDTH - WIDTH / 2.0f;
        starfield[i].Y = SDL_randf() * HEIGHT - HEIGHT / 2.0f;
        starfield[i].Z = 1.0f;
      }
    }

    // draw
    SDL_UpdateTexture(texture, NULL, framebuffer, WIDTH * sizeof(uint32_t));
    SDL_RenderClear(renderer);
    SDL_RenderTexture(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);

    SDL_Delay(16);
  }

  if (!preview_window) {
    SDL_HideWindow(window);
  }

  SDL_DestroyTexture(texture);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return EXIT_SUCCESS;
}