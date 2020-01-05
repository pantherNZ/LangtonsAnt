#pragma once

#include "Component.h"

namespace Reflex::Components
{
	typedef Handle< class Camera > CameraHandle;
	typedef Handle< const class Camera > CameraHandleConst;

	class Camera : public Component, public sf::View
	{
	public:
		Camera( const sf::Vector2f& centre, const sf::Vector2f& size );
		Camera( const sf::FloatRect& viewRect );
		~Camera();

		bool IsActiveCamera() const;
		void SetActiveCamera();

		CameraHandle GetHandle() { return CameraHandle( shared_from_this() ); }
		CameraHandleConst GetHandle() const { return CameraHandleConst( shared_from_this() ); }
	};
}