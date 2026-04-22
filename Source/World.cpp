#include <Book/World.hpp>
#include <Book/Projectile.hpp>
#include <Book/Pickup.hpp>
#include <Book/Foreach.hpp>
#include <Book/TextNode.hpp>
#include <Book/ParticleNode.hpp>
#include <Book/SoundNode.hpp>
#include <Book/Utility.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

#include <algorithm>
#include <cmath>
#include <limits>


World::World(sf::RenderTarget& outputTarget, FontHolder& fonts, SoundPlayer& sounds, State::Difficulty difficulty)
: mTarget(outputTarget)
, mSceneTexture()
, mWorldView(outputTarget.getDefaultView())
, mTextures() 
, mFonts(fonts)
, mSounds(sounds)
, mSceneGraph()
, mSceneLayers()
, mWorldBounds(0.f, 0.f, mWorldView.getSize().x, 5000.f)
, mSpawnPosition(mWorldView.getSize().x / 2.f, mWorldBounds.height - mWorldView.getSize().y / 2.f)
, mScrollSpeed(-50.f)
, mScrollSpeedCompensation(1.f)
, mDifficulty(difficulty)
, mFruitSpawnIndex(0)
, mPlayerCharacters()
, mFruitSpawnPoints()
, mActiveFruits()
, mFinishSprite(nullptr)
{
	mSceneTexture.create(mTarget.getSize().x, mTarget.getSize().y);

	loadTextures();
	buildScene();

	// Prepare the view
	mWorldView.setCenter(mSpawnPosition);
}

void World::setWorldScrollCompensation(float compensation)
{
	mScrollSpeedCompensation = compensation;
}

void World::update(sf::Time dt)
{
	// Scroll the world, reset player velocity
	mWorldView.move(0.f, mScrollSpeed * dt.asSeconds() * mScrollSpeedCompensation);	

	FOREACH(Character* character, mPlayerCharacters)
		character->setVelocity(0.f, 0.f);

	// Setup commands to destroy entities, and guide missiles
	destroyEntitiesOutsideView();
	guideMissiles();

	// Forward commands to scene graph, adapt velocity (scrolling, diagonal correction)
	while (!mCommandQueue.isEmpty())
		mSceneGraph.onCommand(mCommandQueue.pop(), dt);

	adaptPlayerVelocity();

	// Collision detection and response (may destroy entities)
	handleCollisions();

	// Remove characters that were destroyed (World::removeWrecks() only destroys the entities, not the pointers in mPlayerCharacters)
	auto firstToRemove = std::remove_if(mPlayerCharacters.begin(), mPlayerCharacters.end(), std::mem_fn(&Character::isMarkedForRemoval));
	mPlayerCharacters.erase(firstToRemove, mPlayerCharacters.end());

	// Remove all destroyed entities, create new ones
	mSceneGraph.removeWrecks();
	spawnFruits();

	// Regular update step, adapt position (correct if outside view)
	mSceneGraph.update(dt, mCommandQueue);
	adaptPlayerPosition();

	updateSounds();
}

void World::draw()
{
	if (PostEffect::isSupported())
	{
		mSceneTexture.clear();
		mSceneTexture.setView(mWorldView);
		mSceneTexture.draw(mSceneGraph);
		mSceneTexture.display();
		mBloomEffect.apply(mSceneTexture, mTarget);
	}
	else
	{
		mTarget.setView(mWorldView);
		mTarget.draw(mSceneGraph);
	}
}

CommandQueue& World::getCommandQueue()
{
	return mCommandQueue;
}

Character* World::getCharacter(int identifier) const
{
	FOREACH(Character* a, mPlayerCharacters)
	{
		if (a->getIdentifier() == identifier)
			return a;
	}

	return nullptr;
}

void World::removeCharacter(int identifier)
{
	Character* character = getCharacter(identifier);
	if (character)
	{
		character->destroy();
		mPlayerCharacters.erase(std::find(mPlayerCharacters.begin(), mPlayerCharacters.end(), character));
	}
}

Character* World::addCharacter(int identifier)
{
	std::unique_ptr<Character> player(new Character(Character::HelloKitty, mTextures, mFonts, mDifficulty));
	player->setPosition(mWorldView.getCenter());
	player->setIdentifier(identifier);

	mPlayerCharacters.push_back(player.get());
	mSceneLayers[UpperAir]->attachChild(std::move(player));
	return mPlayerCharacters.back();
}

void World::createPickup(sf::Vector2f position, Pickup::Type type)
{	
	std::unique_ptr<Pickup> pickup(new Pickup(type, mTextures));
	pickup->setPosition(position);
	pickup->setVelocity(0.f, 1.f);
	mSceneLayers[UpperAir]->attachChild(std::move(pickup));
}

void World::setCurrentBattleFieldPosition(float lineY)
{
	mWorldView.setCenter(mWorldView.getCenter().x, lineY - mWorldView.getSize().y/2);
	mSpawnPosition.y = mWorldBounds.height; 
}

void World::setWorldHeight(float height)
{
	mWorldBounds.height = height;
}

bool World::hasAlivePlayer() const
{
	return mPlayerCharacters.size() > 0;
}

bool World::hasPlayerReachedEnd() const
{
	if (Character* character = getCharacter(1))
		return !mWorldBounds.contains(character->getPosition());
	else 
		return false;
}

void World::loadTextures()
{
	mTextures.load(Textures::Entities, "Media/Textures/Entities.png");
	mTextures.load(Textures::Jungle, "Media/Textures/Jungle.png");
	mTextures.load(Textures::Explosion, "Media/Textures/Explosion.png");
	mTextures.load(Textures::Particle, "Media/Textures/Particle.png");
	mTextures.load(Textures::FinishLine, "Media/Textures/FinishLine.png");
}

void World::adaptPlayerPosition()
{
	// Keep player's position inside the screen bounds, at least borderDistance units from the border
	sf::FloatRect viewBounds = getViewBounds();
	const float borderDistance = 40.f;

	FOREACH(Character* character, mPlayerCharacters)
	{
		sf::Vector2f position = character->getPosition();
		position.x = std::max(position.x, viewBounds.left + borderDistance);
		position.x = std::min(position.x, viewBounds.left + viewBounds.width - borderDistance);
		position.y = std::max(position.y, viewBounds.top + borderDistance);
		position.y = std::min(position.y, viewBounds.top + viewBounds.height - borderDistance);
		character->setPosition(position);
	}
}

void World::adaptPlayerVelocity()
{
	FOREACH(Character* character, mPlayerCharacters)
	{
		sf::Vector2f velocity = character->getVelocity();

		// If moving diagonally, reduce velocity (to have always same velocity)
		if (velocity.x != 0.f && velocity.y != 0.f)
			character->setVelocity(velocity / std::sqrt(2.f));

		// Add scrolling velocity
		character->accelerate(0.f, mScrollSpeed);
	}
}

bool matchesCategories(SceneNode::Pair& colliders, Category::Type type1, Category::Type type2)
{
	unsigned int category1 = colliders.first->getCategory();
	unsigned int category2 = colliders.second->getCategory();

	// Make sure first pair entry has category type1 and second has type2
	if (type1 & category1 && type2 & category2)
	{
		return true;
	}
	else if (type1 & category2 && type2 & category1)
	{
		std::swap(colliders.first, colliders.second);
		return true;
	}
	else
	{
		return false;
	}
}

void World::handleCollisions()
{
	std::set<SceneNode::Pair> collisionPairs;
	mSceneGraph.checkSceneCollision(mSceneGraph, collisionPairs);

	FOREACH(SceneNode::Pair pair, collisionPairs)
	{
		if (matchesCategories(pair, Category::HelloKitty, Category::Fruit))
	{
			auto& player = static_cast<Character&>(*pair.first);
			auto& enemy = static_cast<Character&>(*pair.second);

			// Collision: Player damage = enemy's remaining HP
			player.damage(enemy.getHitpoints());
			enemy.destroy();
		}

		else if (matchesCategories(pair, Category::HelloKitty, Category::Pickup))
	{
			auto& player = static_cast<Character&>(*pair.first);
			auto& pickup = static_cast<Pickup&>(*pair.second);

			// Apply pickup effect to player, destroy projectile
			pickup.apply(player);
			pickup.destroy();
			player.playLocalSound(mCommandQueue, SoundEffect::CollectPickup);
		}

		else if (matchesCategories(pair, Category::Fruit, Category::HelloKittyProjectile)
			  || matchesCategories(pair, Category::HelloKitty, Category::FruitProjectile))
	{
			auto& character = static_cast<Character&>(*pair.first);
			auto& projectile = static_cast<Projectile&>(*pair.second);

			// Apply projectile damage to character, destroy projectile
			character.damage(projectile.getDamage());
			projectile.destroy();
		}
	}
}

void World::updateSounds()
{
	sf::Vector2f listenerPosition;

	// 0 players (multiplayer mode, until server is connected) -> view center
	if (mPlayerCharacters.empty())
	{
		listenerPosition = mWorldView.getCenter();
	}

	// 1 or more players -> mean position between all characters
	else
	{
		FOREACH(Character* character, mPlayerCharacters)
			listenerPosition += character->getWorldPosition();

		listenerPosition /= static_cast<float>(mPlayerCharacters.size());
	}

	// Set listener's position
	mSounds.setListenerPosition(listenerPosition);

	// Remove unused sounds
	mSounds.removeStoppedSounds();
}

void World::buildScene()
{
	// Initialize the different layers
	for (std::size_t i = 0; i < LayerCount; ++i)
	{
		Category::Type category = (i == LowerAir) ? Category::SceneAirLayer : Category::None;

		SceneNode::Ptr layer(new SceneNode(category));
		mSceneLayers[i] = layer.get();

		mSceneGraph.attachChild(std::move(layer));
	}

	// Prepare the tiled background
	sf::Texture& jungleTexture = mTextures.get(Textures::Jungle);
	jungleTexture.setRepeated(true);

	float viewHeight = mWorldView.getSize().y;
	sf::IntRect textureRect(mWorldBounds);
	textureRect.height += static_cast<int>(viewHeight);
	textureRect.top = static_cast<int>(jungleTexture.getSize().y) - textureRect.height;

	// Add the background sprite to the scene
	std::unique_ptr<SpriteNode> jungleSprite(new SpriteNode(jungleTexture, textureRect));
	jungleSprite->setPosition(mWorldBounds.left, mWorldBounds.top - viewHeight);
	mSceneLayers[Background]->attachChild(std::move(jungleSprite));

	// Add the finish line to the scene
	sf::Texture& finishTexture = mTextures.get(Textures::FinishLine);
	std::unique_ptr<SpriteNode> finishSprite(new SpriteNode(finishTexture));
	finishSprite->setPosition(0.f, -76.f);
	mFinishSprite = finishSprite.get();
	mSceneLayers[Background]->attachChild(std::move(finishSprite));

	// Add particle node to the scene
	std::unique_ptr<ParticleNode> smokeNode(new ParticleNode(Particle::Smoke, mTextures));
	mSceneLayers[LowerAir]->attachChild(std::move(smokeNode));

	// Add propellant particle node to the scene
	std::unique_ptr<ParticleNode> propellantNode(new ParticleNode(Particle::Propellant, mTextures));
	mSceneLayers[LowerAir]->attachChild(std::move(propellantNode));

	// Add sound effect node
	std::unique_ptr<SoundNode> soundNode(new SoundNode(mSounds));
	mSceneGraph.attachChild(std::move(soundNode));

	// Add fruit enemies
	addFruits();
}

void World::addFruits()
{
	// Add fruits to the spawn point container.
	// Simple mode stays sparse and mostly centered.
	// Hard mode spreads farther and uses more simultaneous lanes.
	const float leftLane  = -300.f;
	const float leftMid   = -100.f;
	const float rightMid  =  100.f;
	const float rightLane =  300.f;

	auto addWave = [this] (Character::Type type, float relY, std::initializer_list<float> lanes)
	{
		for (float relX : lanes)
			addFruit(type, relX, relY);
	};

	if (mDifficulty == State::Simple)
	{
		addWave(Character::Apple,   600.f, { 0.f });
		addWave(Character::Banana,  1150.f, { leftMid, rightMid });
		addWave(Character::Apple,   1750.f, { 0.f });
		addWave(Character::Banana,  2350.f, { leftMid, rightMid });
		addWave(Character::Apple,   3050.f, { leftLane, 0.f, rightLane });
		addWave(Character::Banana,  3850.f, { 0.f, rightMid });
	}
	else
	{
		addWave(Character::Apple,   500.f, { 0.f });
		addWave(Character::Apple,   850.f, { leftMid, rightMid });
		addWave(Character::Banana, 1100.f, { 0.f });
		addWave(Character::Banana, 1350.f, { leftMid, rightMid });
		addWave(Character::Apple,  1600.f, { leftLane, 0.f, rightLane });
		addWave(Character::Banana, 1850.f, { leftLane, 0.f, rightLane });
		addWave(Character::Apple,  2150.f, { leftLane, leftMid, rightMid, rightLane });
		addWave(Character::Banana, 2450.f, { leftLane, leftMid, rightMid, rightLane });
		addWave(Character::Apple,  2750.f, { leftMid, rightMid });
		addWave(Character::Banana, 3050.f, { 0.f, rightMid });
		addWave(Character::Apple,  3350.f, { leftLane, 0.f, rightLane });
		addWave(Character::Banana, 3650.f, { leftLane, leftMid, rightMid, rightLane });
		addWave(Character::Apple,  3950.f, { leftLane, leftMid, rightMid, rightLane });
		addWave(Character::Banana, 4250.f, { leftLane, leftMid, rightMid, rightLane });
	}

	sortFruits();
}

void World::sortFruits()
{
	// Sort all fruits according to their y value, such that lower enemies are checked first for spawning
	std::sort(mFruitSpawnPoints.begin(), mFruitSpawnPoints.end(), [] (SpawnPoint lhs, SpawnPoint rhs)
	{
		return lhs.y < rhs.y;
	});
}

void World::addFruit(Character::Type type, float relX, float relY)
{
	if (mDifficulty == State::Simple && (mFruitSpawnIndex % 3 == 2))
	{
		++mFruitSpawnIndex;
		return;
	}

	++mFruitSpawnIndex;
	SpawnPoint spawn(type, mSpawnPosition.x + relX, mSpawnPosition.y - relY);
	mFruitSpawnPoints.push_back(spawn);
}

void World::spawnFruits()
{
	// Spawn all fruits entering the view area (including distance) this frame
	while (!mFruitSpawnPoints.empty()
		&& mFruitSpawnPoints.back().y > getBattlefieldBounds().top)
	{
		SpawnPoint spawn = mFruitSpawnPoints.back();
		
		std::unique_ptr<Character> enemy(new Character(spawn.type, mTextures, mFonts, mDifficulty));
		enemy->setPosition(spawn.x, spawn.y);

		mSceneLayers[UpperAir]->attachChild(std::move(enemy));

		// Fruit is spawned, remove it from the list
		mFruitSpawnPoints.pop_back();
	}
}

void World::destroyEntitiesOutsideView()
{
	Command command;
	command.category = Category::Projectile | Category::Fruit;
	command.action = derivedAction<Entity>([this] (Entity& e, sf::Time)
	{
		if (!getBattlefieldBounds().intersects(e.getBoundingRect()))
			e.remove();
	});

	mCommandQueue.push(command);
}

void World::guideMissiles()
{
	// Setup command that stores all fruits in mActiveFruits
	Command enemyCollector;
	enemyCollector.category = Category::Fruit;
	enemyCollector.action = derivedAction<Character>([this] (Character& enemy, sf::Time)
	{
		if (!enemy.isDestroyed())
			mActiveFruits.push_back(&enemy);
	});

	// Setup command that guides all missiles to the fruit which is currently closest to the player
	Command missileGuider;
	missileGuider.category = Category::HelloKittyProjectile;
	missileGuider.action = derivedAction<Projectile>([this] (Projectile& missile, sf::Time)
	{
		// Ignore unguided bullets
		if (!missile.isGuided())
			return;

		float minDistance = std::numeric_limits<float>::max();
		Character* closestEnemy = nullptr;

		// Find closest fruit
		FOREACH(Character* enemy, mActiveFruits)
		{
			float enemyDistance = distance(missile, *enemy);

			if (enemyDistance < minDistance)
			{
				closestEnemy = enemy;
				minDistance = enemyDistance;
			}
		}

		if (closestEnemy)
			missile.guideTowards(closestEnemy->getWorldPosition());
	});

	// Push commands, reset active enemies
	mCommandQueue.push(enemyCollector);
	mCommandQueue.push(missileGuider);
	mActiveFruits.clear();
}

sf::FloatRect World::getViewBounds() const
{
	return sf::FloatRect(mWorldView.getCenter() - mWorldView.getSize() / 2.f, mWorldView.getSize());
}

sf::FloatRect World::getBattlefieldBounds() const
{
	// Return view bounds + some area at top, where enemies spawn
	sf::FloatRect bounds = getViewBounds();
	bounds.top -= 100.f;
	bounds.height += 100.f;

	return bounds;
}
