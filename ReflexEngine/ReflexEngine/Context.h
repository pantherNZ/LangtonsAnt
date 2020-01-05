#pragma once

#include "ResourceManager.h"

namespace Reflex::Core
{
	// Context struct (passed around to access window & various useful managers)
	struct Context
	{
		Context( sf::RenderWindow& window, TextureManager& textureManager, FontManager& fontManager )
			: window( window )
			, textureManager( textureManager )
			, fontManager( fontManager )
		{
		}

		sf::RenderWindow& window;
		TextureManager& textureManager;
		FontManager& fontManager;
	};
}