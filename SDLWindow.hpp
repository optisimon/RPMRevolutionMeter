/*
 * SDLWindow.hpp
 *
 *  Created on: Jun 19, 2016
 *
 *  Copyright (c) 2016 Simon Gustafsson (www.optisimon.com)
 *  Do whatever you like with this code, but please refer to me as the original author.
 */

#pragma once

#include <SDL/SDL.h>
#include <SDL/SDL_gfxPrimitives.h>
#include <SDL/SDL_gfxPrimitives_font.h>

#include <iostream>

#include <assert.h>

class SDLWindow {
public:
	SDLWindow() :
		_should_call_sdl_quit(true),
		_isFullscreen(false),
		_smallSizeWidth(800),
		_smallSizeHeight(500 + _top_text_height),
		_w(_smallSizeWidth),
		_h(_smallSizeHeight),
		_screen(0)
	{
		if ( SDL_Init(SDL_INIT_VIDEO) < 0 )
		{
			printf("Unable to init SDL: %s\n", SDL_GetError());
			exit(1);
		}

		const SDL_VideoInfo * nativeMode = SDL_GetVideoInfo();

		// TODO: Use this later on...
		std::cout
		<< "# SDL Native resolution and mode suggestion: w*h="
		<< nativeMode->current_w << "x" << nativeMode->current_h
		<< ", bpp=" << int(nativeMode->vfmt->BitsPerPixel) << "\n";

		_fullscreenWidth = nativeMode->current_w;
		_fullscreenHeight = nativeMode->current_h;

		_screen = SDL_SetVideoMode(_w, _h, 32, SDL_HWSURFACE/*|SDL_DOUBLEBUF*/);
		if ( _screen == NULL )
		{
			printf("Unable to set %dx%d video: %s\n", _w, _h, SDL_GetError());
			exit(1);
		}
		SDL_WM_SetCaption("RPM Revolution Meter", "RPM Meter");
		clear();
	}

	~SDLWindow()
	{
		if (_should_call_sdl_quit)
			SDL_Quit();
	}

	int getWidth() { return _w; }

	int getHeight() { return _h; }

	bool isFullscreen() { return _isFullscreen; }

	void setFullscreenMode(bool shouldGoFullscreen = true)
	{
		Uint32 flags = 0;
		if (shouldGoFullscreen)
		{
			_w = _fullscreenWidth;
			_h = _fullscreenHeight;
			flags = SDL_HWSURFACE | SDL_FULLSCREEN /*|SDL_DOUBLEBUF*/;
		}
		else
		{
			_w = _smallSizeWidth;
			_h = _smallSizeHeight;
			flags = SDL_HWSURFACE /*|SDL_DOUBLEBUF*/;
		}

		std::cout << "# Requested: " << _w << "x" << _h << "\n";
		_screen = SDL_SetVideoMode(_w, _h, 32, flags);
		if ( _screen == NULL )
		{
			printf("Unable to set %dx%d video: %s\n", _w, _h, SDL_GetError());
			exit(1);
		}

		_isFullscreen = shouldGoFullscreen;
	}

	void blank()
	{
		clearInternalFramebuffer(_screen);
		SDL_Flip(_screen);
		clearInternalFramebuffer(_screen);
	}

	void clear()
	{
		clearInternalFramebuffer(_screen);
	}

	void drawTopText()
	{
		stringRGBA(
				_screen,
				0,
				0,
				"ESC = quit, F11 = toggle fullscreen, +/- = change digit size, f = toggle rpm averaging",
				255, 255, 255, 255);
	}

	void drawAdditionalStats(
			float rpm,
			float filteredRpm,
			int signalMax,
			int signalMin,
			int threshold,
			int hysteresis)
	{
		char buff[100];

		int dy = _top_text_height + 1;
		int x = _w - 200;
		int y = dy;

		snprintf(buff, sizeof(buff), "          RPM: %5.1f", rpm);
		stringRGBA(	_screen, x, y, buff, 255, 255, 255, 255);
		y += dy;
		snprintf(buff, sizeof(buff), " filtered RPM: %5.1f", filteredRpm);
		stringRGBA(	_screen, x, y, buff, 255, 255, 255, 255);
		y += dy;
		snprintf(buff, sizeof(buff), "sensor maxval: %5d", signalMax);
		stringRGBA(	_screen, x, y, buff, 255, 255, 255, 255);
		y += dy;
		snprintf(buff, sizeof(buff), "sensor minval: %5d", signalMin);
		stringRGBA(	_screen, x, y, buff, 255, 255, 255, 255);
		y += dy;
		snprintf(buff, sizeof(buff), "sensor thresh: %5d", threshold);
		stringRGBA(	_screen, x, y, buff, 255, 255, 255, 255);
		y += dy;
		snprintf(buff, sizeof(buff), "sensor hyster: %5d", hysteresis);
		stringRGBA(	_screen, x, y, buff, 255, 255, 255, 255);

	}


	void drawLine(Sint16 x1, Sint16 y1,
			Sint16 x2, Sint16 y2,
			Uint8 r, Uint8 g, Uint8 b, Uint8 a)
	{
		lineRGBA(_screen, x1, y1, x2, y2, r, g, b, a);
	}


	void drawDigits(int number, int maxNumDigits, bool showZeros, bool showUnlitSegments, int digitScaling = 3)
	{
		char str[100];
		snprintf(str, sizeof(str), "%d", number);

		int numNeededCharacters = strlen(str);

		int numEmptyCharacters = maxNumDigits - numNeededCharacters;
		int x = 10;
		int y = _top_text_height*2;

		if (showZeros)
		{
			if ( str[0] == '-')
			{
				drawDigit(x, y, -1, digitScaling, showUnlitSegments);
				char* p = str;
				while (*p)
				{
					*p = *(p+1);
					p++;

				}
				x += getDigitWidthInterdistance(digitScaling);
				numNeededCharacters--;
			}
		}

		for (int i = 0; i < numEmptyCharacters; i++)
		{
			if (showZeros)
			{
				drawDigit(x, y, 0, digitScaling, showUnlitSegments);
			}
			x += getDigitWidthInterdistance(digitScaling);
		}

		for (int i = 0; i < numNeededCharacters; i++)
		{
			if ( str[i] == '-')
			{
				drawDigit(x, y, -1, digitScaling, showUnlitSegments);
			}
			else
			{
				drawDigit(x, y, str[i] - '0', digitScaling, showUnlitSegments);
			}
			x += getDigitWidthInterdistance(digitScaling);
		}
	}

	int getDigitWidthInterdistance (int scaling)
	{
		const uint16_t h_seg_w = 15 * scaling;
		const uint16_t v_seg_w = 5 * scaling;

		return h_seg_w + v_seg_w;
	}

	int getDigitHeightInterdistance (int scaling)
	{
		const uint16_t h_seg_h = 5 * scaling;
		const uint16_t v_seg_w = 5 * scaling;
		const uint16_t v_seg_h = 25 * scaling;
		const uint16_t v_separation = scaling;

		return 2*h_seg_h + 2*v_seg_h + 4*v_separation + v_seg_w;
	}

	void drawDigit(int x, int y, int digit, int digitScaling = 3, bool showUnlitSegments = false)
	{
		enum {
			a = 1,
			b = 2,
			c = 4,
			d = 8,
			e = 16,
			f = 32,
			g = 64,
		};
		uint8_t segments = digitTo7Segment(digit);

		//   0  _______
		//   9 |___a___|
		//      _     _
		//  12 | |   | |
		//     |f|   |b|
		//  32 |_|   |_|
		//  35  _______
		//  44 |___g___|
		//      _     _
		//  47 | |   | |
		//     |e|   |c|
		//  67 |_|   |_|
		//  70  _______
		//  80 |___d___|
		//     0 9  20 29 <-- Typical X coordinates

		const uint16_t scaling = digitScaling;
		const uint16_t h_seg_h = 5 * scaling;
		const uint16_t h_seg_w = 15 * scaling;
		const uint16_t v_seg_h = 25 * scaling;
		const uint16_t v_seg_w = 5 * scaling;
		const uint16_t v_separation =  scaling;


		SDL_Rect segmentLUT[] = {
				// x, y, w, h
				{ 0,  0, h_seg_w, h_seg_h},
				{int16_t(h_seg_w - v_seg_w), int16_t(h_seg_h + v_separation), v_seg_w, v_seg_h},
				{int16_t(h_seg_w - v_seg_w), int16_t(2*h_seg_h + v_seg_h + 3*v_separation), v_seg_w, v_seg_h},
				{ 0, int16_t(2*h_seg_h + 2*v_seg_h + 4*v_separation), h_seg_w, h_seg_h},
				{ 0, int16_t(2*h_seg_h + v_seg_h + 3*v_separation), v_seg_w, v_seg_h},
				{ 0, int16_t(h_seg_h + v_separation), v_seg_w, v_seg_h},
				{ 0, int16_t(h_seg_h + v_seg_h + 2*v_separation), h_seg_w, h_seg_h}
		};
//		SDL_Rect segmentLUT[] = {
//				// x, y, w, h
//				{ 0,  0, 30, 10},
//				{20, 12, 10, 20},
//				{20, 47, 10, 20},
//				{ 0, 70, 30, 10},
//				{ 0, 47, 10, 20},
//				{ 0, 12, 10, 20},
//				{ 0, 35, 30, 10}
//		};

		for (int i = 1, n=0; i < 128; i*=2, n++)
		{
			Uint32 color;
			int R = 64;
			int G = 255;
			int B = 64;			
			if (segments & i)
			{
				color = SDL_MapRGB(_screen->format, R, G, B);
			}
			else
			{
				color = SDL_MapRGB(_screen->format, R/32, G/32, B/32);
			}
			
			SDL_Rect rect = segmentLUT[n];
			rect.x += x;
			rect.y += y;
			
			if ((segments & i) || showUnlitSegments)
			{
				SDL_FillRect(_screen, &rect, color);
			}
		}
	}

	void flip()
	{
		SDL_Flip(_screen);
	}

private:
	bool _should_call_sdl_quit;
	bool _isFullscreen;
	int _smallSizeWidth;
	int _smallSizeHeight;
	int _w;
	int _h;
	int _fullscreenWidth;
	int _fullscreenHeight;
	enum { _top_text_height = 10 };
	SDL_Surface *_screen;

	/**
	 *   __a__
	 *  |     |
	 * f|     |b
	 *  |__g__|
	 *  |     |
	 * e|     |c
	 *  |__d__|
	 *
	 *
	 *
	 */
	uint8_t digitTo7Segment(int digit)
	{
		assert(digit >= -1);
		assert(digit <= 9);
		enum {
			a = 1,
			b = 2,
			c = 4,
			d = 8,
			e = 16,
			f = 32,
			g = 64,
		};
		uint8_t LUT[] = {
				a|b|c|d|e|f,
				b|c,
				a|b|g|e|d,
				a|g|d|b|c,
				f|g|b|c,
				a|f|g|c|d,
				a|c|d|e|f|g,
				f|a|b|c,
				a|b|c|d|e|f|g,
				a|b|c|d|f|g,
		};

		if (digit < 0)
		{
			return g;
		}
		else
		{
			return LUT[digit];
		}
	}

	void lock(SDL_Surface *screen)
	{
		if ( SDL_MUSTLOCK(screen) )
		{
			if ( SDL_LockSurface(screen) < 0 )
			{
				return;
			}
		}
	}

	void unlock(SDL_Surface *screen)
	{
		if ( SDL_MUSTLOCK(screen) )
		{
			SDL_UnlockSurface(screen);
		}
	}

	void safeDrawPixel(SDL_Surface *screen, int x, int y,
			Uint8 R, Uint8 G, Uint8 B)
	{
		if( x >= 0 && x < _w && y >= 0 && y < _h)
		{
			drawPixel(screen, x, y, R, G, B);
		}
	}

	void drawPixel(SDL_Surface *screen, int x, int y,
			Uint8 R, Uint8 G, Uint8 B)
	{
		Uint32 color = SDL_MapRGB(screen->format, R, G, B);
		switch (screen->format->BytesPerPixel)
		{
		case 1: // Assuming 8-bpp
		{
			Uint8 *bufp;
			bufp = (Uint8 *)screen->pixels + y*screen->pitch + x;
			*bufp = color;
		}
		break;
		case 2: // Probably 15-bpp or 16-bpp
		{
			Uint16 *bufp;
			bufp = (Uint16 *)screen->pixels + y*screen->pitch/2 + x;
			*bufp = color;
		}
		break;
		case 3: // Slow 24-bpp mode, usually not used
		{
			Uint8 *bufp;
			bufp = (Uint8 *)screen->pixels + y*screen->pitch + x * 3;
			if(SDL_BYTEORDER == SDL_LIL_ENDIAN)
			{
				bufp[0] = color;
				bufp[1] = color >> 8;
				bufp[2] = color >> 16;
			} else {
				bufp[2] = color;
				bufp[1] = color >> 8;
				bufp[0] = color >> 16;
			}
		}
		break;
		case 4: // Probably 32-bpp
		{
			Uint32 *bufp;
			bufp = (Uint32 *)screen->pixels + y*screen->pitch/4 + x;
			*bufp = color;
		}
		break;
		}
	}

	void getTopLeftOffset(int& posx, int& posy, int width_rgb, int height_rgb)
	{
		int free_x = _w - width_rgb;
		int free_y = _h - height_rgb - _top_text_height;

		assert(free_x >= 0);
		assert(free_y >= 0);

		posy = _top_text_height + free_y/2;
		posx = free_x/2;
	}

	void drawBayerAsRGB_Internal(unsigned char* img, int width_bayer, int height_bayer)
	{
		assert(width_bayer/2 <= _w);
		assert(height_bayer/2 <= _h);

		int posx, posy;
		getTopLeftOffset(posx, posy, width_bayer/2, height_bayer/2);

		for (int y = 0; y < height_bayer/2; y++)
		{
			for (int x = 0; x < width_bayer/2; x++)
			{
				uint8_t R  = img[(y*2  )*width_bayer + x*2 + 0];
				uint8_t G1 = img[(y*2  )*width_bayer + x*2 + 1];
				uint8_t G2 = img[(y*2+1)*width_bayer + x*2 + 0];
				uint8_t B =  img[(y*2+1)*width_bayer + x*2 + 1];
				safeDrawPixel(_screen, x + posx, y + posy, R, (G1 + G2)/2, B);
			}
		}
	}


	/**
	 * Clears the internal frame buffer, but does NOT flip buffers
	 */
	void clearInternalFramebuffer(SDL_Surface *screen)
	{
		//lock(screen);
		memset(screen->pixels, 0, screen->pitch*_h);
		//unlock(screen);
	}


};
