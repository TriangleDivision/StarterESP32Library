#ifndef STATE_MANAGER_H
#define STATE_MANAGER_H

#include <Arduino.h>

#define MAX_STATES 10

template <typename StateEnum>
class StateManager {
public:
    typedef void (*StateFunction)();

    struct State {
        StateEnum state;
        StateFunction onEnter;
        StateFunction onUpdate;
        StateFunction onExit;
    };

    StateManager() : currentStateIndex(-1), stateCount(0), debugMode(false), stateStartTime(0) {}

    void setDebugMode(bool debug) {
        debugMode = debug;
    }

    void registerState(StateEnum state, StateFunction onEnter, StateFunction onUpdate, StateFunction onExit) {
        if (stateCount < MAX_STATES) {
            State& s = states[stateCount++];
            s.state = state;
            s.onEnter = onEnter;
            s.onUpdate = onUpdate;
            s.onExit = onExit;
        } else {
            setStatus("Error: Maximum number of states reached", true);
        }
    }

    void setState(StateEnum state) {
        if (state == getState()) {
          setStatus("Warning: Already in state, not changing.", false);
          return;
        }

        for (int i = 0; i < stateCount; i++) {
            if (states[i].state == state) {
                if (currentStateIndex >= 0 && states[currentStateIndex].onExit) {
                    states[currentStateIndex].onExit();
                }

                currentStateIndex = i;
                stateStartTime = millis();
                setStatus("State changed to: " + String(static_cast<int>(state)));
                if (states[currentStateIndex].onEnter) {
                    states[currentStateIndex].onEnter();
                }
                return;
            }
        }

        setStatus("Error: Unknown state", true);
    }

    StateEnum getState() const {
        if (currentStateIndex >= 0) {
            return states[currentStateIndex].state;
        }
        return static_cast<StateEnum>(-1); // Return an invalid state
    }

    void update() {
        if (currentStateIndex >= 0 && states[currentStateIndex].onUpdate) {
            states[currentStateIndex].onUpdate();
        }
    }

    unsigned long getElapsedTime() const {
        return millis() - stateStartTime;
    }

private:
    State states[MAX_STATES];
    int stateCount;
    int currentStateIndex;
    bool debugMode;
    String lastStatus;
    unsigned long stateStartTime;

    void setStatus(const String& status, bool isError = false) {
        lastStatus = status;
        if (debugMode) {
            Serial.println(status);
        }
    }
};

#endif // STATE_MANAGER_H
