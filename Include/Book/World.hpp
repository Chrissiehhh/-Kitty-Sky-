#ifndef BOOK_WORLD_HPP
#define BOOK_WORLD_HPP

#include <Book/ResourceHolder.hpp>
#include <Book/ResourceIdentifiers.hpp>
#include <Book/SceneNode.hpp>
#include <Book/SpriteNode.hpp>
#include <Book/Character.hpp>
#include <Book/CommandQueue.hpp>
#include <Book/Command.hpp>
#include <Book/Pickup.hpp>
#include <Book/BloomEffect.hpp>
#include <Book/SoundPlayer.hpp>
#include <Book/State.hpp>

#include <SFML/System/NonCopyable.hpp>
#include <SFML/Graphics/View.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <array>
#include <queue>


// Forward declaration
namespace sf
{
	class RenderTarget;
}

class World : private sf::NonCopyable
{
	public:
											World(sf::RenderTarget& outputTarget, FontHolder& fonts, SoundPlayer& sounds, State::Difficulty difficulty);
		void								update(sf::Time dt);
		void								draw();

		sf::FloatRect						getViewBounds() const;		
		CommandQueue&						getCommandQueue();
		Character*							addCharacter(int identifier);
		void								removeCharacter(int identifier);
		void								setCurrentBattleFieldPosition(float lineY);
		void								setWorldHeight(float height);

		void								addFruit(Character::Type type, float relX, float relY);
		void								sortFruits();

		bool 								hasAlivePlayer() const;
		bool 								hasPlayerReachedEnd() const;

		void								setWorldScrollCompensation(float compensation);

		Character*							getCharacter(int identifier) const;
		sf::FloatRect						getBattlefieldBounds() const;

		void								createPickup(sf::Vector2f position, Pickup::Type type);


	private:
		void								loadTextures();
		void								adaptPlayerPosition();
		void								adaptPlayerVelocity();
		void								handleCollisions();
		void								updateSounds();

		void								buildScene();
		void								addFruits();
		void								spawnFruits();
		void								destroyEntitiesOutsideView();
		void								guideMissiles();


	private:
		enum Layer
		{
			Background,
			LowerAir,
			UpperAir,
			LayerCount
		};

		struct SpawnPoint 
		{
			SpawnPoint(Character::Type type, float x, float y)
			: type(type)
			, x(x)
			, y(y)
			{
			}

			Character::Type type;
			float x;
			float y;
		};


	private:
		sf::RenderTarget&					mTarget;
		sf::RenderTexture					mSceneTexture;
		sf::View							mWorldView;
		TextureHolder						mTextures;
		FontHolder&							mFonts;
		SoundPlayer&						mSounds;

		SceneNode							mSceneGraph;
		std::array<SceneNode*, LayerCount>	mSceneLayers;
		CommandQueue						mCommandQueue;

		sf::FloatRect						mWorldBounds;
		sf::Vector2f						mSpawnPosition;
		float								mScrollSpeed;
		float								mScrollSpeedCompensation;
		State::Difficulty					mDifficulty;
		std::size_t							mFruitSpawnIndex;
		std::vector<Character*>				mPlayerCharacters;

		std::vector<SpawnPoint>				mFruitSpawnPoints;
		std::vector<Character*>				mActiveFruits;

		BloomEffect							mBloomEffect;

		SpriteNode*							mFinishSprite;
};

#endif // BOOK_WORLD_HPP
