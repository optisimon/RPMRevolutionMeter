/*
 * SDLEventHandler.hpp
 *
 *  Created on: Jun 19, 2016
 *
 *  Copyright (c) 2016 Simon Gustafsson (www.optisimon.com)
 *  Do whatever you like with this code, but please refer to me as the original author.
 */

#pragma once

#include <SDL/SDL.h>


class SDLEventHandler {
public:
	SDLEventHandler() :
		_should_quit(false),
		_should_go_fullscreen(false),
		_should_display_filtered_rpm(false),
		_digitScaling(2),
		_thresholdPercentage(50)
	{
		refresh();
	}

	void refresh()
	{
		SDL_Event event;

		while ( SDL_PollEvent(&event) )
		{
			if ( event.type == SDL_QUIT )
			{
				_should_quit = true;
			}

			if ( event.type == SDL_KEYDOWN )
			{
				switch(event.key.keysym.sym)
				{
				case SDLK_ESCAPE:
					_should_quit = true;
					break;

				case SDLK_F11:
					_should_go_fullscreen ^= true;
					break;

				case SDLK_PLUS:
				case SDLK_KP_PLUS:
					_digitScaling = std::min<int>(8, _digitScaling + 1);
					break;

				case SDLK_MINUS:
				case SDLK_KP_MINUS:
					_digitScaling = std::max<int>(1, _digitScaling - 1);
					break;

				case SDLK_UP:
					_thresholdPercentage += 5;
					if (_thresholdPercentage > _maxAllowedThresholdPercentage)
					{
						_thresholdPercentage = _maxAllowedThresholdPercentage;
					}
					break;

				case SDLK_DOWN:
					_thresholdPercentage -= 5;
					if (_thresholdPercentage < _minAllowedThresholdPercentage)
					{
						_thresholdPercentage = _minAllowedThresholdPercentage;
					}
					break;

				case SDLK_f:
					_should_display_filtered_rpm ^= 1;
					break;

				case SDLK_SPACE:
					break;
				default:
					break;
				}
			}

			if (event.type == SDL_VIDEORESIZE)
			{
				int w = event.resize.w;
				int h = event.resize.h;
				std::cout
				<< "Resize events not supported. (resize to "
				<< w << "x" << h << " requested)\n";
			}
		}
	}

	bool shouldQuit() const { return _should_quit; }

	bool shouldGoFullscreen() const { return _should_go_fullscreen; }

	bool shouldDisplayFilteredRPM() const { return _should_display_filtered_rpm; }

	int getDigitScaling() const { return _digitScaling; }

	void setDigitScaling(int scaling) { _digitScaling = scaling; }

	int getThresholdPercentage() const { return _thresholdPercentage; }

	void setThresholdPercentage(int thresholdPercentage)
	{
		assert(thresholdPercentage >= _minAllowedThresholdPercentage);
		assert(thresholdPercentage <= _maxAllowedThresholdPercentage);
		_thresholdPercentage = thresholdPercentage;
	}

private:
	bool _should_quit;
	bool _should_go_fullscreen;
	bool _should_display_filtered_rpm;

	int _digitScaling;
	int _thresholdPercentage;

	static const int _minAllowedThresholdPercentage = 5;
	static const int _maxAllowedThresholdPercentage = 95;
};
