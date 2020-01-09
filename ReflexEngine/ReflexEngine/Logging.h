#pragma once

// Includes
#include "VectorMap.h"

namespace Reflex
{
	// Logging system
	inline void LOG( const std::string& log ) { std::cout << log << "\n"; }

	#define LOG_CRIT( x ) { Reflex::LOG( Stream( "CRIT: (" << __FUNCTION__ << ") " << x ) ); }
	#define LOG_WARN( x ) Reflex::LOG( Stream( "Warning: (" << __FUNCTION__ << ") " << x ) );
	#define LOG_INFO( x ) Reflex::LOG( Stream( "Info: " << x ) );
	#define THROW( x ) throw std::runtime_error( Stream( "EXCEPTION: (" << __FUNCTION__ << ") " << x ) );

	// Profiling code
	namespace Core
	{
		class Profiler : sf::NonCopyable
		{
		public:
			static Profiler& GetProfiler();
			void StartProfile( const std::string& name );
			void EndProfile( const std::string& name );
			void FrameTick( const sf::Int64 frameTimeMS );
			void OutputResults( const std::string& file );

		protected:
			Profiler() { }

		private:
			struct ProfileData
			{
				sf::Clock timer;
				sf::Int64 currentFrame = 0;
				sf::Int64 shortestFrame = std::numeric_limits< int >::infinity();
				sf::Int64 longestFrame = 0;
				sf::Int64 totalDuration = 0;
				unsigned minHitCount = 1U;
				unsigned maxHitCount = 1U;
				unsigned currentHitCount = 1U;
				unsigned totalSamples = 0U;
			};

			sf::Int64 m_totalDuration = 0;

			Reflex::VectorMap< std::string, ProfileData > m_profileData;
			static std::unique_ptr< Profiler > s_profiler;
		};

		class ScopedProfiler : sf::NonCopyable
		{
		public:
			ScopedProfiler( const std::string& name );
			~ScopedProfiler();

		private:
			const std::string m_profileName;
		};
	}

	#define PROFILE Reflex::Core::ScopedProfiler profile( __FUNCTION__ );
	#define PROFILE_NAME( x ) Reflex::Core::ScopedProfiler profile( x );
}