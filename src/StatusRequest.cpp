#include "TaskScheduler.h"

#ifdef _TASK_STATUS_REQUEST

StatusRequest::StatusRequest() {iCount = 0; iStatus = 0; }
void StatusRequest::setWaiting(unsigned int aCount = 1) { iCount = aCount; iStatus = 0; }
bool StatusRequest::pending() { return (iCount != 0); }
bool StatusRequest::completed() { return (iCount == 0); }
int StatusRequest::getStatus() { return iStatus; }

/** Signals completion of the StatusRequest by one of the participating events
 *  @param: aStatus - if provided, sets the return code of the StatusRequest: negative = error, 0 (default) = OK, positive = OK with a specific status code
 *  Negative status will complete Status Request fully (since an error occured).
 *  @return: true, if StatusRequest is complete, false otherwise (still waiting for other events)
 */
bool StatusRequest::signal(int aStatus) {
	if ( iCount) {	// do not update the status request if it was already completed
		if (iCount > 0)  --iCount; 
		if ( (iStatus = aStatus) < 0 ) iCount = 0;   // if an error is reported, the status is requested to be completed immediately
	}
	return (iCount == 0); 
}

void StatusRequest::signalComplete(int aStatus) {
	if (iCount) { // do not update the status request if it was already completed
		iCount = 0; 
		iStatus = aStatus;
	}
}

#endif  // _TASK_STATUS_REQUEST