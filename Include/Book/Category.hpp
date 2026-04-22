#ifndef BOOK_CATEGORY_HPP
#define BOOK_CATEGORY_HPP


// Entity/scene node category, used to dispatch commands
namespace Category
{
	enum Type
	{
		None				= 0,
		SceneAirLayer		= 1 << 0,
		HelloKitty			= 1 << 1,
		Fruit				= 1 << 2,
		Pickup				= 1 << 3,
		HelloKittyProjectile = 1 << 4,
		FruitProjectile		= 1 << 5,
		ParticleSystem		= 1 << 6,
		SoundEffect			= 1 << 7,

		Character = HelloKitty | Fruit,
		Projectile = HelloKittyProjectile | FruitProjectile,
	};
}

#endif // BOOK_CATEGORY_HPP
