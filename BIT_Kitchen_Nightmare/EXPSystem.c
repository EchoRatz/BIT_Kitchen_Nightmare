/*#include "EXPSystem.h"

void initializeExpSystem(ExpSystem* expSystem) {
    expSystem->currentExp = 0;
    expSystem->expToNextLevel = 100; // Example value
    expSystem->currentLevel = 1;
}

void gainExp(ExpSystem* expSystem, int expGained) {
    expSystem->currentExp += expGained;
    while (expSystem->currentExp >= expSystem->expToNextLevel) {
        expSystem->currentExp -= expSystem->expToNextLevel;
        expSystem->currentLevel++;
        expSystem->expToNextLevel = calculateExpForNextLevel(expSystem->currentLevel);
        // Implement calculateExpForNextLevel function based on your game's leveling curve
    }
}*/