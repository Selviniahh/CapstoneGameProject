#pragma once

namespace ETG
{
    //Registers every reflectable type (engine bases + all game types) into TypeRegistry.
    //Lives on the game side so the engine never includes game headers; GameManager calls
    //this once at startup, right after the editor UI is initialized.
    void RegisterGameTypes();
}
