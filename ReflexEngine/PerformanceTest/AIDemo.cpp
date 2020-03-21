// Includes
#include "AIDemo.h"
#include "../ReflexEngine/CameraComponent.h"
#include "../ReflexEngine/SFMLObjectComponent.h"
#include "../ReflexEngine/TransformComponent.h"
#include "../ReflexEngine/SteeringSystem.h"

// Entry point
int main()
{
	Engine engine( "AI Steering Demo" );
	engine.RegisterState< GameState >( true );
	engine.Run();

	return 0;
}

GameState::GameState( StateManager& stateManager, Context context )
	: State( stateManager, context )
{
	auto camera = GetWorld().CreateObject()->AddComponent< Reflex::Components::Camera >( Reflex::Vector2uToVector2f( context.window.getSize() ) / 2.0f, Reflex::Vector2uToVector2f( context.window.getSize() ) );
	camera->EnableWASDPanning( sf::Vector2f( 300.0f, 300.0f ) );
	camera->EnableArrowKeyPanning();
	camera->EnableMouseZooming( 1.2f, true );

	for( unsigned i = 0; i < 10000; ++i )
		CreateBoid( GetWorld().RandomWindowPosition() )->Wander( wanderCircleRadius, wanderCircleDistance, wanderAngleDelta, maxVelocity );
}

Reflex::Components::SteeringHandle GameState::CreateBoid( const sf::Vector2f& pos )
{
	auto newBoid = GetWorld().CreateObject( pos );
	auto boid = newBoid->AddComponent< Reflex::Components::Steering >();

	auto circle = newBoid->AddComponent< Reflex::Components::SFMLObject >( sf::CircleShape( 10.0f, 30 ) );
	circle->GetCircleShape().setFillColor( sf::Color::Red );
	
	return boid;
}

void GameState::Render()
{
	ImGui::SetNextWindowPos( sf::Vector2( 5.0f, 0.17f * GetWorld().GetWindow().getSize().y ), ImGuiCond_::ImGuiCond_Once );
	ImGui::Begin( "AI Demo", nullptr, ImGuiWindowFlags_AlwaysAutoResize );

	bool changed = ImGui::SliderFloat( "Max Velocity", &maxVelocity, 0.0f, 1000.0f );
	changed = ImGui::SliderFloat( "Slowing Radius", &slowingRadius, 0.0f, 1000.0f ) || changed;

	ImGui::NewLine();

	changed = ImGui::SliderFloat( "Wander Circle Radius", &wanderCircleRadius, 1.0f, 1000.0f ) || changed;
	changed = ImGui::SliderFloat( "Wander Circle Distance", &wanderCircleDistance, 0.0f, 1000.0f ) || changed;
	changed = ImGui::SliderAngle( "Wander Angle Delta", &wanderAngleDelta ) || changed;

	const bool reset = ImGui::Button( "Reset" );

	if( reset || changed )
	{
		//GetWorld().GetSystem< Reflex::Systems::SteeringSystem >()->ForEachObject< Reflex::Components::Steering >(
		//	[&]( const Reflex::Components::Steering::Handle& boid )
		//	{
		//		if( reset )
		//			boid->GetTransform()->setPosition( GetWorld().RandomWindowPosition() );
		//
		//		if( changed )
		//			boid->Wander( wanderCircleRadius, wanderCircleDistance, wanderAngleDelta, maxVelocity );
		//	} );
	}

	ImGui::End();
}
