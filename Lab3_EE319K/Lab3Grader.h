#include <stdint.h>

enum TExaSmode{
  SCOPE,
  LOGICANALYZER,
	GRADER,
	NONE
};

// ************TExaS_Init*****************
// Initialize scope or logic analyzer, triggered by periodic timer
// This needs to be called once
// Inputs: Scope or Logic analyzer
// Outputs: none
void TExaS_Init(enum TExaSmode mode);

// ************TExaS_Stop*****************
// Stop the transfer
// Inputs:  none
// Outputs: none
void TExaS_Stop(void);
