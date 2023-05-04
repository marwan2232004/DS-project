#include "FCFS.h"
#include "../Process Scheduler/Process Scheduler.h"

FCFS::FCFS(Scheduler* Sched)
	:Processor(Sched) {}


void FCFS::ScheduleAlgo() {
	if (State == IDLE && RDY_LIST.RemoveHead(R)) {

		R->AddWaitingTime(S->Get_TimeStep() - R->GetLastRunTime());  // This Line and the next one should be added to all processors  
		R->SetLastRunTime(S->Get_TimeStep());

		State = BUSY;
		if (!R->GetResponseTime()) R->SetResponseTime(S->Get_TimeStep());
		R->SetState(RUn);
		R->SetProcessor(this);

		QFT -= R->GetCPUTime();

		if (S->Get_NR()) {
			while (R->GetCurrWaitingTime() > S->Get_MaxW()) {

				S->FCFSMigration(R);

				if (RDY_LIST.RemoveHead(R)) {
					State = BUSY;
					if (!R->GetResponseTime()) R->SetResponseTime(S->Get_TimeStep());
					R->SetState(RUn);
					R->SetProcessor(this);
					QFT -= R->GetCPUTime();
				}
				else { State = IDLE; break; }
			}
		}
	}
	if (State == BUSY) {

		TBT++;
		R->UpdateInfo();

		if (!R->GetCPUTime())
			S->TO_TRM(R);
		else if (R->GetIO() && !R->GetIO()->getFirst())
				S->TO_BLK(R);

	}
	else TIT++;
}

void FCFS::AddProcess(Process* process) {
	UpdateState();
	process->SetProcessor(this);
	RDY_LIST.InsertEnd(process);
	QFT += process->GetCPUTime();
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
		S->TO_TRM(R);
		return;
	}
	for (int i = 0; i < RDY_LIST.size(); i++) {
		RDY_LIST.GetItem(i, p);
		if (p->GetPID() == PID) {
			RDY_LIST.Remove(i, p);
			QFT -= p->GetCPUTime();
			S->TO_TRM(p);
			return;
		}
	}
}
void FCFS::Lose(Process*& Stolen) {
	if (!RDY_LIST.RemoveHead(Stolen)) Stolen = nullptr;
	else QFT -= Stolen->GetCPUTime();
}
