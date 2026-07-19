#pragma once

namespace ETG
{
    class GameManager;

    //Level content lives here, not in GameManager: adding a new gun/enemy/item to the world
    //means touching only this class. GameManager stays a content-agnostic loop driver.
    class SpawnInitialLevel
    {
    public:
        //Spawn order matters: Hero has to update before the guns/enemies that read its state,
        //and PendingSpawns preserves insertion order when flushed into WorldObjects.
        static void Spawn(GameManager& game);
    };
}
