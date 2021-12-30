#include <iostream>
#include <conio.h>
#include <vector>
using namespace std;

struct Place {
	string id;
	int token;
	int maxToken = INT_MAX;
	Place(string name, int initToken, int n) {
		token = initToken;
		id = name;
		maxToken = n;
	}
	void setToken(int diff) { token += diff; }
	int getToken() { return token; }
	string getID() { return id; }
};

struct Trans {
	string id;
	vector<int> input;
	vector<int> output;
	Trans(string name, vector<int> in, vector<int> out) {
		id = name; 
		input = in;
		output = out;
	}
	string getID() { return id; }
};

struct TR {
	vector<int> pre;
	vector<int> post;
	vector<string> sequence;
	TR () { }
};

struct PetriNet {
private:
	vector<Place> places;
	vector<Trans> trans;
	vector<int> initialMarking;
	vector<int> canFire;
	vector<vector<int>> markingLog;
	vector<string> firingSequence;
	vector<TR> reachable;

	bool canFireAt(Trans t) {
		bool isEnabled = true, isFull = false;
		for (int i = 0; i < t.input.size(); i++) {
			Place p = places[t.input[i]];
			if (p.getToken() < 1) {
				isEnabled = false;
				break;
			}
		}
		for (int i = 0; i < t.output.size(); i++) {
			Place p = places[t.output[i]];
			if (p.getToken() == p.maxToken) {
				isFull = true;
				break;
			}
		}
		return (isEnabled && (!isFull));
	}

	void updateCanFire() {
		canFire.clear();
		for (int i = 0; i < trans.size(); i++) {
			if (canFireAt(trans[i])) canFire.push_back(i);
		}
	}

	void fire(Trans t) {
		for (int i = 0; i < t.input.size(); i++) {
			places[t.input[i]].setToken(-1);
		}
		for (int i = 0; i < t.output.size(); i++) {
			places[t.output[i]].setToken(1);
		}
	}

	vector<int> captureMarking() {
		vector<int> marking;
		for (Place p : places) {
			marking.push_back(p.getToken());
		}
		return marking;
	}

	bool isExist(vector<int> currentMarking) {
		for (int i = 0; i < reachable.size(); i++) {
			int count = 0;
			for (int j = 0; j < currentMarking.size(); j++) {
				if (currentMarking[j] != reachable[i].pre[j]) break;
				else count++;
			}
			if (count == reachable[i].pre.size()) return true;
		}
		return false;
	}

	void getTry(PetriNet net) {
		for (int i = 0; i < net.canFire.size(); i++) {
			TR stateChanging;
			stateChanging.pre = net.captureMarking();
			net.fire(net.trans[net.canFire[i]]);
			stateChanging.post = net.captureMarking();
			firingSequence.push_back(net.trans[net.canFire[i]].getID());
			stateChanging.sequence = firingSequence;
			reachable.push_back(stateChanging);
			net.updateCanFire();
			if (net.canFire.size() && !isExist(stateChanging.post)) {
				getTry(net);
				firingSequence.pop_back();
			}
		}
	}

	PetriNet(){ }

public:

	PetriNet(vector<vector<string>> component, vector<vector<int>> relation, vector<int> maxToken, vector<int> init_Marking) {
		int t = relation.size(), p = relation[0].size();

		for (int i = 0; i < init_Marking.size(); i++) {
			if (init_Marking[i] > maxToken[i]) init_Marking[i] = maxToken[i];
		}

		initialMarking = init_Marking;
		markingLog.push_back(init_Marking);

		for (int i = 0; i < p; i++) {
			Place pi = Place(component[0][i], initialMarking[i], maxToken[i]);
			places.push_back(pi);
		}

		for (int i = 0; i < t; i++) {
			vector<int> in, out;
			for (int j = 0; j < p; j++) {
				if (relation[i][j] == 1) in.push_back(j);
				if (relation[i][j] == -1) out.push_back(j);
				if (relation[i][j] == 2) {
					in.push_back(j);
					out.push_back(j);
				}
			}
			Trans ti = Trans(component[1][i], in, out);
			trans.push_back(ti);
		}
	}

	void reset() {
		for (int i = 0; i < places.size(); i++) {
			places[i].setToken(-places[i].getToken() + initialMarking[i]);
		}
		markingLog.clear();
		markingLog.push_back(initialMarking);
	}

	void printPlaces() {
		for (Place p : places) cout << p.getID() << " ";
	}

	void printTrans() {
		for (Trans t : trans) cout << t.getID() << " ";
	}

	void printMarkingLog() {
		for (int i = 0; i < markingLog.size(); i++) {
			if (i == 0) cout << "Initial Marking: " << endl;
			if (i > 0) cout << endl << "Firing Transition: " << firingSequence[i - 1] << endl;
			for (int j = 0; j < markingLog[i].size(); j++) {
				cout << places[j].getID() << ": " << markingLog[i][j] << "; ";
			}
			cout << endl;
		}
	}

	void PrintMarkingCount() {
		cout << markingLog.size();
	}

	void run() {
		updateCanFire();
		while (canFire.size()) {
			for (int i = 0; i < canFire.size(); i++) {
				fire(trans[canFire[i]]);
				markingLog.push_back(captureMarking());
				firingSequence.push_back(trans[canFire[i]].getID());
			}
			updateCanFire();
		}
	}

	void run(int maxFiring) {
		updateCanFire();
		int count = 0;
		while (canFire.size() && (maxFiring - count)) {
			for (int i = 0; i < canFire.size(); i++) {
				fire(trans[canFire[i]]);
				markingLog.push_back(captureMarking());
				firingSequence.push_back(trans[canFire[i]].getID());
			}
			updateCanFire();
			count++;
		}
	}

	void findReachable() {
		PetriNet net;
		net.places = places;
		net.trans = trans;
		net.initialMarking = initialMarking;
		net.reset();
		net.updateCanFire();
		getTry(net);

		if (!reachable.size()) cout << "No reachable marking from M0!";
		else {
			for (int i = 0; i < reachable.size(); i++) {
				cout << "Firing Sequence: ";
				for (int j = 0; j < reachable[i].sequence.size(); j++) {
					cout << reachable[i].sequence[j] << " ";
				}
				cout << endl;
				cout << "(";
				for (int j = 0; j < reachable[i].pre.size(); j++) {
					cout << reachable[i].pre[j] << " ";
				}
				cout << ")->(";
				for (int j = 0; j < reachable[i].pre.size(); j++) {
					cout << reachable[i].post[j] << " ";
				}
				cout << ")" << endl;
			}
		}
	}
};

int main() {
	int max = INT_MAX;

	vector<vector<string>> component = { {"wait", "free", "busy", "inside", "docu", "done"},
										{ "start", "change", "end" } };
	vector<vector<int>> relation = { {1, 1, -1, -1, 0, 0},
									 {0, 0, 1, 1, -1, -1},
									 {0, -1, 0, 0, 1, 0} };
	vector<int> maxToken = {max, max, max, max, max, max };
	vector<int> init_marking = {4, 1, 0, 0, 0, 0};

	//vector<vector<string>> component = { {"wait", "inside", "done"},
	//								{ "start", "change"} };
	//vector<int> maxToken = { max, max, max };
	//vector<vector<int>> relation = { {1, -1, 0},
	//								 {0, 1, -1} };
	//vector<int> init_marking = { 5, 0, 1 };

	//vector<vector<string>> component = { { "free", "busy", "docu"},
	//									{ "start", "change", "end"} };
	//vector<int> maxToken = { max, max, max };
	//vector<vector<int>> relation = { {1, -1, 0},
	//								 {0, 1, -1},
	//								 {-1, 0, 1} };
	//vector<int> init_marking = { 1, 0, 0};

	PetriNet net = PetriNet(component, relation, maxToken, init_marking);
	//net.printPlaces();
	//cout << endl;
	//net.printTrans();

	//net.findReachable();

	net.run();
	net.printMarkingLog();
	cout << endl << "Total Marking: ";
	net.PrintMarkingCount();

	_getch();
}