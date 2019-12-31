#pragma once

#include "ResourceManager.h"

namespace Reflex
{
	namespace Core
	{
		// Context struct (passed around to access window & various useful managers)
		struct Context
		{
			Context( sf::RenderWindow& _window, TextureManager& _textureManager, FontManager& _fontManager )
				: window( _window )
				, textureManager( _textureManager )
				, fontManager( _fontManager )
			{
			}

			sf::RenderWindow& window;
			TextureManager& textureManager;
			FontManager& fontManager;
		};
	}
}