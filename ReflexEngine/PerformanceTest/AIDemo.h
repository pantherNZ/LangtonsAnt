// Includes
#include "../ReflexEngine/Engine.h"
#include "../ReflexEngine/SteeringComponent.h"

using namespace Reflex::Core;

class GameState : public State
{
public:
	GameState( StateManager& stateManager, Context context );

protected:
	Reflex::Components::SteeringHandle CreateBoid( const sf::Vector2f& pos );
	void Render() override;

protected:
	// Data Members
	Reflex::Components::CameraHandle camera;

	float maxVelocity = 50.0f;
	float slowingRadius = 0.0f;
	float wanderCircleRadius = 1.0f;
	float wanderCircleDistance = 0.0f;
	float wanderAngleDelta = 0.0f;
};