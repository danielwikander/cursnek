#ifndef HHSCORE
#define HHSCORE
#include <string>
typedef struct hhscore {
	int hscore;
	std::string hsname;
	bool operator<(const hhscore &rhs) const { return hscore > rhs.hscore; }
} hhscore;
#endif
