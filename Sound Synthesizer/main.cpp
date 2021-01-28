#include <iostream>
#include "olcNoiseMaker.h"

using namespace std;


atomic<double> dFrequencyOutput = 0.0;

// Fucntion that makes the sound (in Hrz -> note A = 440 Hrz)
double MakeNoise(double dTime) {

	double dOutput = 1.0 * sin(dFrequencyOutput * 2 * 3.14159 * dTime);

	if (dOutput > 0.0)
		return 0.2;
	else
		return -0.2;
}


int main() {

	wcout << "------ Synthesizer ------" << endl;

	// Get all sound hardware
	vector<wstring> devices = olcNoiseMaker<short>::Enumerate();

	// Display findings
	for (auto i : devices) wcout << "Found Output Device: " << i << endl;

	// Create sound machine
	olcNoiseMaker<short> sound(devices[0], 44100, 1, 8, 512);

	// Link noise function with sound machine
	sound.SetUserFunction(MakeNoise);

	double dOctaveBaseFrequency = 110.0; // A2
	double d12hRootof2 = pow(2.0, 1.0 / 12.0);

	while (1) {
		// Add a keyboard
		bool bKeyPressed = false;
		for (int i = 0; i < 15; ++i) {
			if (GetAsyncKeyState((unsigned char)("ZSXCFVGBNJMK\xbcL\xbe"[i])) & 0x8000) {
				// 0->A,  1->A#, 2->B, ...., 12->A2
				dFrequencyOutput = dOctaveBaseFrequency * pow(d12hRootof2, i);
				bKeyPressed = true;
			}
		}

		if (!bKeyPressed) {
			dFrequencyOutput = 0.0;
		}
	}


	return 0;
}
