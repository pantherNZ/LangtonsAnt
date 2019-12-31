#pragma once

#include "Utility.h"
#include "Logging.h"

namespace Reflex
{
	enum class ResourceID : unsigned short;

	namespace Core
	{
		template< typename Resource >
		class ResouceManager;

		typedef ResouceManager< sf::Texture > TextureManager;
		typedef ResouceManager< sf::Font > FontManager;

		template< typename Resource >
		class ResouceManager
		{
		public:
			// Resource loading
			Resource& LoadResource( const ResourceID id, const std::string& filename );

			// Resource loading with a second parameter (for shaders, or textures that specify a bounding rect etc.)
			template< typename Parameter >
			Resource& LoadResource( const ResourceID id, const std::string& filename, const Parameter& secondParam );

			// Fetches a resource from the map
			const Resource& GetResource( const ResourceID id ) const;

		private:
			// Private helper function to insert a new resource into the map and do error checking
			Resource& InsertResource( const ResourceID id, const std::string& filename, std::unique_ptr< Resource > newResource );

			std::map< const ResourceID, std::unique_ptr< Resource > > m_resourceMap;
		};

		// Template functions
		template< typename Resource >
		Resource& ResouceManager< Resource >::LoadResource( const ResourceID id, const std::string& filename )
		{
			auto newResource = std::make_unique< Resource >();

			if( !newResource->loadFromFile( filename ) )
				THROW( "Failed to load " << filename );

			return InsertResource( id, filename, std::move( newResource ) );
		}

		template< typename Resource >
		template< typename Parameter >
		Resource& ResouceManager< Resource >::LoadResource( const ResourceID id, const std::string& filename, const Parameter& secondParam )
		{
			auto newResource = std::make_unique< Resource >();

			if( !newResource->loadFromFile( filename, secondParam ) )
				THROW( "Failed to load " << filename );

			return InsertResource( id, filename, std::move( newResource ) );
		}

		template< typename Resource >
		const Resource& ResouceManager< Resource >::GetResource( const ResourceID id ) const
		{
			auto found = m_resourceMap.find( id );

			if( found == m_resourceMap.end() )
				LOG_CRIT( "Resource doesn't exist " << ( int )id );

			return *found->second;
		}

		template< typename Resource >
		Resource& ResouceManager< Resource >::InsertResource( const ResourceID id, const std::string& filename, std::unique_ptr< Resource > newResource )
		{
			auto inserted = m_resourceMap.insert( std::make_pair( id, std::move( newResource ) ) );

			//if( !inserted.second )
			//	LOG_CRIT( "Resource already loaded " << filename );

			auto& resource_iter = *inserted.first;
			return *resource_iter.second;
		}
	}
}
