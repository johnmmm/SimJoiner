#include "SimJoiner.h"

using namespace std;

int main(int argc, char **argv) {
    SimJoiner joiner;

    vector<EDJoinResult> resultED;
    vector<JaccardJoinResult> resultJaccard;

    unsigned edThreshold = 3;
    double jaccardThreshold = 0.75;

    joiner.joinJaccard(argv[1], argv[2], jaccardThreshold, resultJaccard);
    //joiner.joinED(argv[1], argv[2], edThreshold, resultED);

    return 0;
}
