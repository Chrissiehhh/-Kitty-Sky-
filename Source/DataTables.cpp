#include <Book/DataTables.hpp>
#include <Book/Character.hpp>
#include <Book/Projectile.hpp>
#include <Book/Pickup.hpp>
#include <Book/Particle.hpp>


// For std::bind() placeholders _1, _2, ...
using namespace std::placeholders;

std::vector<CharacterData> initializeCharacterData()
{
	std::vector<CharacterData> data(Character::TypeCount);

	data[Character::HelloKitty].hitpoints = 150;
	data[Character::HelloKitty].speed = 200.f;
	data[Character::HelloKitty].fireInterval = sf::seconds(1);
	data[Character::HelloKitty].texture = Textures::Entities;
	data[Character::HelloKitty].textureRect = sf::IntRect(3, 4, 48, 52);
	data[Character::HelloKitty].hasRollAnimation = true;

	data[Character::Apple].hitpoints = 20;
	data[Character::Apple].speed = 80.f;
	data[Character::Apple].texture = Textures::Entities;
	data[Character::Apple].textureRect = sf::IntRect(158, 4, 65, 54);
	data[Character::Apple].directions.push_back(Direction(+45.f, 80.f));
	data[Character::Apple].directions.push_back(Direction(-45.f, 160.f));
	data[Character::Apple].directions.push_back(Direction(+45.f, 80.f));
	data[Character::Apple].fireInterval = sf::Time::Zero;
	data[Character::Apple].hasRollAnimation = false;

	data[Character::Banana].hitpoints = 40;
	data[Character::Banana].speed = 50.f;
	data[Character::Banana].texture = Textures::Entities;
	data[Character::Banana].textureRect = sf::IntRect(230, 4, 56, 52);
	data[Character::Banana].directions.push_back(Direction(+45.f,  50.f));
	data[Character::Banana].directions.push_back(Direction(  0.f,  50.f));
	data[Character::Banana].directions.push_back(Direction(-45.f, 100.f));
	data[Character::Banana].directions.push_back(Direction(  0.f,  50.f));
	data[Character::Banana].directions.push_back(Direction(+45.f,  50.f));
	data[Character::Banana].fireInterval = sf::seconds(2);
	data[Character::Banana].hasRollAnimation = false;

	data[Character::PineappleBoss].hitpoints = 500;
	data[Character::PineappleBoss].speed = 85.f;
	data[Character::PineappleBoss].texture = Textures::PineappleBoss;
	data[Character::PineappleBoss].textureRect = sf::IntRect();
	data[Character::PineappleBoss].directions.push_back(Direction(+75.f, 220.f));
	data[Character::PineappleBoss].directions.push_back(Direction(-75.f, 440.f));
	data[Character::PineappleBoss].directions.push_back(Direction(+75.f, 220.f));
	data[Character::PineappleBoss].fireInterval = sf::seconds(1.2f);
	data[Character::PineappleBoss].hasRollAnimation = false;

	return data;
}

std::vector<ProjectileData> initializeProjectileData()
{
	std::vector<ProjectileData> data(Projectile::TypeCount);

	data[Projectile::HelloKittyBullet].damage = 10;
	data[Projectile::HelloKittyBullet].speed = 300.f;
	data[Projectile::HelloKittyBullet].texture = Textures::Entities;
	data[Projectile::HelloKittyBullet].textureRect = sf::IntRect(163, 67, 25, 20);

	data[Projectile::FruitBullet].damage = 10;
	data[Projectile::FruitBullet].speed = 300.f;
	data[Projectile::FruitBullet].texture = Textures::Entities;
	data[Projectile::FruitBullet].textureRect = sf::IntRect(215, 62, 22, 25);

	data[Projectile::Missile].damage = 200;
	data[Projectile::Missile].speed = 150.f;
	data[Projectile::Missile].texture = Textures::Entities;
	data[Projectile::Missile].textureRect = sf::IntRect(44, 71, 25, 24);

	data[Projectile::Bomb].damage = 260;
	data[Projectile::Bomb].speed = 220.f;
	data[Projectile::Bomb].texture = Textures::BombPickup;
	data[Projectile::Bomb].textureRect = sf::IntRect();

	return data;
}

std::vector<PickupData> initializePickupData()
{
	std::vector<PickupData> data(Pickup::TypeCount);
	
	data[Pickup::HealthRefill].texture = Textures::Entities;
	data[Pickup::HealthRefill].textureRect = sf::IntRect(5, 68, 30, 24);
	data[Pickup::HealthRefill].action = [] (Character& a) { a.repair(25); };
	
	data[Pickup::MissileRefill].texture = Textures::Entities;
	data[Pickup::MissileRefill].textureRect = sf::IntRect(44, 71, 25, 24);
	data[Pickup::MissileRefill].action = std::bind(&Character::collectMissiles, _1, 3);
	
	data[Pickup::FireSpread].texture = Textures::Entities;
	data[Pickup::FireSpread].textureRect = sf::IntRect(83, 68, 25, 26);
	data[Pickup::FireSpread].action = std::bind(&Character::increaseSpread, _1);
	
	data[Pickup::FireRate].texture = Textures::Entities;
	data[Pickup::FireRate].textureRect = sf::IntRect(126, 69, 23, 25);
	data[Pickup::FireRate].action = std::bind(&Character::increaseFireRate, _1);

	data[Pickup::BombCrate].texture = Textures::BombPickup;
	data[Pickup::BombCrate].textureRect = sf::IntRect();
	data[Pickup::BombCrate].action = [] (Character& a)
	{
		a.collectBombs(1);
		a.repair(15);
	};

	return data;
}

std::vector<ParticleData> initializeParticleData()
{
	std::vector<ParticleData> data(Particle::ParticleCount);

	data[Particle::Propellant].color = sf::Color(255, 255, 50);
	data[Particle::Propellant].lifetime = sf::seconds(0.6f);

	data[Particle::Smoke].color = sf::Color(50, 50, 50);
	data[Particle::Smoke].lifetime = sf::seconds(4.f);

	return data;
}
