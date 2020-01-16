#pragma once

#include "Component.h"
#include "CameraSystem.h"
#include "Object.h"

namespace Reflex::Components
{
	DefineComponentWithInheritence( Camera, sf::View )
		friend class Reflex::Systems::CameraSystem;

		Camera( const sf::Vector2f& centre, const sf::Vector2f& size, const bool activate = false );
		Camera( const sf::FloatRect& viewRect );
		~Camera();

		void OnConstructionComplete() final;

		bool IsActiveCamera() const;
		void SetActiveCamera();

		//void ApplyShake( const float force );
		void FollowObject( const Reflex::ObjectHandleConst object, const float interpolationSpeed = 0.0f );
		bool IsFollowingObject() const { return followTarget.IsValid(); }

		void EnableMousePanning( const sf::Vector2f& margin, const sf::Vector2f& speed = sf::Vector2f( 0.0f, 0.0f ) );
		void DisableMousePanning() { flags.reset( MousePanning ); }

		void EnableMouseZooming( const float scaleFactor = 0.0f, const bool centreOnMouse = false );
		void DisableMouseZooming() { flags.reset( MouseZooming ); }

		void EnableWASDPanning( const sf::Vector2f& speed = sf::Vector2f( 0.0f, 0.0f ) );
		void DisableWASDPanning() { flags.reset( WASDPanning ); }

		void EnableArrowKeyPanning( const sf::Vector2f& speed = sf::Vector2f( 0.0f, 0.0f ) );
		void DisableArrowKeyPanning() { flags.reset( ArrowPanning ); }

		void SetPanSpeed( const sf::Vector2f& speed ) { panSpeed = speed; }
		sf::Vector2f GetPanSpeed() const { return panSpeed; }
		void SetZoomScaleFactor( const float speed ) { zoomScaleFactor = speed; }
		float GetZoomScaleFactor() const { return zoomScaleFactor; }

		void SetAdditivePanning( const bool additive ) { flags[AdditivePanning] = additive; }
		void SetNormaliseDiagonalPanning( const bool normalise ) { flags[NormaliseDiagonalPanning] = normalise; }

	protected:
		enum Flags
		{
			MousePanning,
			WASDPanning,
			ArrowPanning,
			MouseZooming,
			ZoomCentreOnMouse,
			AdditivePanning,
			NormaliseDiagonalPanning,
			StartActivated,
			NumFlags,
		};

		std::bitset< NumFlags > flags;
		float zoomScaleFactor = 1.0f;
		sf::Vector2f panSpeed;
		sf::Vector2f panMouseMargin;
		float followInterpSpeed = 0.0f;
		ObjectHandleConst followTarget;
	};
}