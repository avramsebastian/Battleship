#pragma once

#ifdef BATTLESHIPLOGIC_EXPORTS
#define BATTLESHIP_API __declspec(dllexport)
#else
#define BATTLESHIP_API __declspec(dllimport)
#endif