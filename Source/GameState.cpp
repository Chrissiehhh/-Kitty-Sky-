#include <Book/GameState.hpp>
#include <Book/MusicPlayer.hpp>
#include <Book/Utility.hpp>

#include <SFML/Graphics/RenderWindow.hpp>


GameState::GameState(StateStack& stack, Context context)
: State(stack, context)
, mWorld(*context.window, *context.fonts, *context.sounds, *context.difficulty)
, mPlayer(1, context.keys1)
, mDifficultyText()
{
	mWorld.addCharacter(1);
	mPlayer.setMissionStatus(Player::MissionRunning);

	mDifficultyText.setFont(context.fonts->get(Fonts::Main));
	mDifficultyText.setCharacterSize(14u);
	mDifficultyText.setFillColor(sf::Color::White);
	mDifficultyText.setPosition(5.f, 5.f);
	mDifficultyText.setString(*context.difficulty == State::Simple ? "Mode: Simple" : "Mode: Hard");

	// Play game theme
	context.music->play(Music::MissionTheme);
}

void GameState::draw()
{
	mWorld.draw();

	sf::RenderWindow& window = *getContext().window;
	window.setView(window.getDefaultView());
	window.draw(mDifficultyText);
}

bool GameState::update(sf::Time dt)
{
	mWorld.update(dt);

	if (!mWorld.hasAlivePlayer())
	{
		mPlayer.setMissionStatus(Player::MissionFailure);
		requestStackPush(States::GameOver);
	}
	else if (mWorld.hasPlayerReachedEnd())
	{
		mPlayer.setMissionStatus(Player::MissionSuccess);
		requestStackPush(States::MissionSuccess);
	}

	CommandQueue& commands = mWorld.getCommandQueue();
	mPlayer.handleRealtimeInput(commands);

	return true;
}

bool GameState::handleEvent(const sf::Event& event)
{
	// Game input handling
	CommandQueue& commands = mWorld.getCommandQueue();
	mPlayer.handleEvent(event, commands);

	// Escape pressed, trigger the pause screen
	if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)
		requestStackPush(States::Pause);

	return true;
}
