#include "FCFS.h"
#include "../Process Scheduler/Process Scheduler.h"

FCFS::FCFS(Scheduler* Sched , int n)
	:Processor(Sched,n) {}


void FCFS::ScheduleAlgo() {
	if (State == IDLE && RDY_LIST.RemoveHead(R)) {

		R->AddWaitingTime(S->Get_TimeStep() - R->GetLastRunTime());  // This Line and the next one should be added to all processors  

		State = BUSY;
		if (!R->GetResponseTime()) R->SetResponseTime(S->Get_TimeStep());
		R->SetState(RUn);

		QFT -= R->GetCPURemainingTime();

		FCFSMigration(); //check for Migration Process First 
	}
	if (State == BUSY) {

		TBT++;
		R->UpdateInfo();

		if (!R->GetCPURemainingTime()) {
			R->SetChildrenState(); //set children to orphans
			S->TO_TRM(R);
		}
		else if (R->GetIO() && !R->GetIO()->getFirst())
			S->TO_BLK(R);

	}
	else if (State == STOP) {
		if (N_TEMP == N) {
			if (R) {
				R->SetProcessor(nullptr);
				S->TO_SHORTEST_RDY(R, true);
			}
			while (RDY_LIST.RemoveHead(R))
				S->TO_SHORTEST_RDY(R, true);
			R = nullptr;
			QFT = 0;
		}
		else if (!N_TEMP) {
			State = IDLE;
			N_TEMP = N;
			return;
		}
		N_TEMP--;
	}
	else TIT++;
}

void FCFS::RemoveOrphans() {
	if (R && R->GetState() == ORPH) {
		QFT -= R->GetCPURemainingTime();
		S->TO_TRM(R);
	}
	for (int i = 0; i < RDY_LIST.size(); i++) {
		Process* p = nullptr;
		RDY_LIST.GetItem(i, p);
		if (p->GetState() == ORPH) {
			QFT -= p->GetCPURemainingTime(); //need to discuss
			RDY_LIST.Remove(i, p);
			S->TO_TRM(p);
			i--;
		}
	}
}

void FCFS::AddProcess(Process* process) {
	UpdateState();
	process->SetProcessor(this);
	RDY_LIST.InsertEnd(process);
	QFT += process->GetCPURemainingTime();
}
void FCFS::Print() {
	Output* pOut = S->getOutput();
	pOut->PrintOut("Processor " + to_string(ID));
	pOut->PrintOut("[FCFS]: " + to_string(RDY_LIST.size()) + " RDY: ");
	RDY_LIST.print();
}
void FCFS::Kill(int PID) {
	Process* p = nullptr;
	if (State == BUSY && R->GetPID() == PID) {
		R->SetChildrenState();
		S->TO_TRM(R);
		return;
	}
	for (int i = 0; i < RDY_LIST.size(); i++) {
		RDY_LIST.GetItem(i, p);
		if (p->GetPID() == PID) {
			RDY_LIST.Remove(i, p);
			QFT -= p->GetCPURemainingTime();
			p->SetChildrenState();
			S->TO_TRM(p);
			return;
		}
	}
}

void FCFS::FCFSMigration() {

	if (S->Get_NR() && !R->GetParent()) { //Don't enter if no RR exists or process is a child because children must be in fcfs only
		S->DecideShortestSpecific(2);
		if (!S->GetSRR()) return; //return if the RR processors are OverHeated
		while (R->GetCurrWaitingTime() > S->Get_MaxW()) { //Migrate Multiple Processes in the same time step until a process have Waiting Time less than MaxW

			S->FCFSMigration(R);

			if (RDY_LIST.RemoveHead(R)) {
				if (!R->GetResponseTime()) R->SetResponseTime(S->Get_TimeStep());
				R->SetProcessor(this);
				R->SetState(RUn);
				State = BUSY;
				QFT -= R->GetCPURemainingTime();
			}
			else break;
		}
	}
}

void FCFS::Forking() {
	int FP = S->Get_FP();
	int Rand = 1 + rand() % 100;
	if (R && Rand <= FP && (!R->GetLeftChild() || !R->GetRightChild())) {
		Process* child = S->AddChildToSQ(R->GetArrivalTime(), R->GetCPURemainingTime());
		if (!R->GetLeftChild()) {
			R->SetLeftChild(child);
		}
		else if (!R->GetRightChild()) {
			R->SetRightChild(child);
		}
	}
}

void FCFS::Lose(Process*& Stolen) {
	Process* p = nullptr;
	for (int i = 0; i < RDY_LIST.size(); i++) {
		RDY_LIST.GetItem(i, p);
		if (!p->GetParent() && p->GetState() != ORPH) {
			RDY_LIST.Remove(i, p);
			QFT -= p->GetCPURemainingTime();
			p->SetProcessor(nullptr);
			Stolen = p;
			break;
		}
	}
}
