#pragma once

// Attack Decay Sustain Release
struct sEnvelopeADSR {
	double dAttackTime;
	double dDecayTime;
	double dReleaseTime;

	double dSustainAmplitude;
	double dStartAmplitude;

	double dTriggerOnTime;
	double dTriggerOffTime;

	bool bNoteOn;

	sEnvelopeADSR() {
		dAttackTime = 0.05;
		dDecayTime = 0.01;
		dReleaseTime = 0.20;
		dSustainAmplitude = 0.8;
		dStartAmplitude = 1.0;
		dTriggerOnTime = 0.0;
		dTriggerOffTime = 0.0;
		bNoteOn = false;
	}

	double getAmplitude(double dTime) {
		double dAmplitude = 0.0;
		double dLifeTime = dTime - dTriggerOnTime;

		if (bNoteOn) {
			// ADS
			// Attack
			if (dLifeTime <= dAttackTime)
				dAmplitude = (dLifeTime / dAttackTime) * dStartAmplitude;

			// Decay
			if (dLifeTime > dAttackTime && dLifeTime <= (dAttackTime + dDecayTime))
				dAmplitude = ((dLifeTime - dAttackTime) / dDecayTime) * (dSustainAmplitude - dStartAmplitude) + dStartAmplitude;

			//Sustain
			if (dLifeTime > (dAttackTime + dDecayTime))
				dAmplitude = dSustainAmplitude;
		}
		else {
			// Release
			dAmplitude = ((dTime - dTriggerOffTime) / dReleaseTime) * (0.0 - dSustainAmplitude) + dSustainAmplitude;

		}

		if (dAmplitude <= 0.0001) {
			dAmplitude = 0.0;
		}

		return dAmplitude;
	}

	void NoteOn(double dTimeOn) {
		dTriggerOnTime = dTimeOn;
		bNoteOn = true;
	}

	void NoteOff(double dTimeOff) {
		dTriggerOffTime = dTimeOff;
		bNoteOn = false;
	}
};