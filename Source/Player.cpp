#include <Book/Player.hpp>
#include <Book/CommandQueue.hpp>
#include <Book/Character.hpp>
#include <Book/Foreach.hpp>

#include <map>

using namespace std::placeholders;

struct CharacterMover
{
	CharacterMover(float vx, float vy, int identifier)
	: velocity(vx, vy)
	, characterID(identifier)
	{
	}

	void operator() (Character& character, sf::Time) const
	{
		if (character.getIdentifier() == characterID)
			character.accelerate(velocity * character.getMaxSpeed());
	}

	sf::Vector2f velocity;
	int characterID;
};

struct CharacterFireTrigger
{
	CharacterFireTrigger(int identifier)
	: characterID(identifier)
	{
	}

	void operator() (Character& character, sf::Time) const
	{
		if (character.getIdentifier() == characterID)
			character.fire();
	}

	int characterID;
};

struct CharacterMissileTrigger
{
	CharacterMissileTrigger(int identifier)
	: characterID(identifier)
	{
	}

	void operator() (Character& character, sf::Time) const
	{
		if (character.getIdentifier() == characterID)
			character.launchMissile();
	}

	int characterID;
};


Player::Player(sf::Int32 identifier, const KeyBinding* binding)
: mKeyBinding(binding)
, mCurrentMissionStatus(MissionRunning)
, mIdentifier(identifier)
{
	// Set initial action bindings
	initializeActions();

// Assign all categories to the player's HelloKitty
FOREACH(auto& pair, mActionBinding)
	pair.second.category = Category::HelloKitty;
}

void Player::handleEvent(const sf::Event& event, CommandQueue& commands)
{
	if (event.type == sf::Event::KeyPressed)
	{
		Action action;
		if (mKeyBinding && mKeyBinding->checkAction(event.key.code, action) && !isRealtimeAction(action))
			commands.push(mActionBinding[action]);
	}
}

void Player::handleRealtimeInput(CommandQueue& commands)
{
	if (!mKeyBinding)
		return;

	std::vector<Action> activeActions = mKeyBinding->getRealtimeActions();
	FOREACH(Action action, activeActions)
		commands.push(mActionBinding[action]);
}

void Player::setMissionStatus(MissionStatus status)
{
	mCurrentMissionStatus = status;
}

Player::MissionStatus Player::getMissionStatus() const
{
	return mCurrentMissionStatus;
}

void Player::initializeActions()
{
	mActionBinding[PlayerAction::MoveLeft].action      = derivedAction<Character>(CharacterMover(-1,  0, mIdentifier));	
	mActionBinding[PlayerAction::MoveRight].action     = derivedAction<Character>(CharacterMover(+1,  0, mIdentifier));
	mActionBinding[PlayerAction::MoveUp].action        = derivedAction<Character>(CharacterMover( 0, -1, mIdentifier));
	mActionBinding[PlayerAction::MoveDown].action      = derivedAction<Character>(CharacterMover( 0, +1, mIdentifier));
	mActionBinding[PlayerAction::Fire].action          = derivedAction<Character>(CharacterFireTrigger(mIdentifier));
	mActionBinding[PlayerAction::LaunchMissile].action = derivedAction<Character>(CharacterMissileTrigger(mIdentifier));
}
