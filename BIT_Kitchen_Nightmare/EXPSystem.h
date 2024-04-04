// exp_system.h
#ifndef EXP_SYSTEM_H
#define EXP_SYSTEM_H

typedef struct {
    int currentExp;
    int expToNextLevel;
    int currentLevel;
} ExpSystem;

void initializeExpSystem(ExpSystem* expSystem);
void gainExp(ExpSystem* expSystem, int expGained);
// Additional declarations as needed

#endif