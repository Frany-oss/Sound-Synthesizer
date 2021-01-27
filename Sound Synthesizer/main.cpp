#include <iostream>
#include "olcNoiseMaker.h"

using namespace std;


int main() {

	wcout << "------ Synthesizer ------" << endl;

	// Get all sound hardware
	vector<wstring> devices = olcNoiseMaker<short>::Enumerate();

	// Display findings
	for (auto i : devices) wcout << "Found Output Device: " << i << endl;

	// Create sound machine
	olcNoiseMaker<short> sound(devices[0]);

	return 0;
}
