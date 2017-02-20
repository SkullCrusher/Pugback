#ifndef __PUGBACK_CALLBACKS_H
#define __PUGBACK_CALLBACKS_H

#include <vector>
#include <string> 
#include <iostream>

	// The results from a callback.
#define PUG_RESULTS_SUCCESS 1    // Success.
#define PUG_RESULTS_FAILURE 2    // Failure.
#define PUG_RESULTS_INVALID 3    // Invalid settings or unable to call it back.
#define PUG_RESULTS_DISABLED 4	 // Used when the callback is disabled.
#define PUG_RESULTS_CREATEDNEW 5 // Used with groups when one does not exist it will create new one.

	// Define the threading, this enables Mutex to prevent two threads editing the stack at the same time.
#define PUG_ENABLE_THREADING


	// If multi-threading is enabled include the required libraries.
#ifdef PUG_ENABLE_THREADING         
	#include <mutex>
#endif 


	// A helper function to get the value from void *.
template<class T>
T PugGetValue(void * arg) {
	return *((T*)arg);
}


	// A simple call back object that is used by the pug engine to call it back.
class PUG_CallBack {

		// Used to reference the callback.
	private: int ID;

		// Used by the engine to determine if the callback should get called or not.
	private: int GroupID;

		// Used when printing out information about the callback.
	private: std::string Name;

		// If the callback is disabled.
	private: bool Disabled;

		// The callback function.
	private: int (*CallbackFunc)(void*);
			

		// Getters and setters.
	public: void SetID(int argID) {
		ID = argID;
	}
	public: int  GetID() {
		return ID;
	}

	public: void SetGroupID(int argGroupID) {
		GroupID = argGroupID;
	}
	public: int  GetGroupID() {
		return GroupID;
	}

	public: void SetName(std::string argName) {
		Name = argName;
	}
	public: std::string GetName() {
		return Name;
	}

	public: void SetCallback(int arg(void *a)) {
		CallbackFunc = arg;
	}


		// The do the callback.
	public: int DoCallback(void *arg) {

		// If the callback is disabled, returned.
		if (Disabled) {
			return PUG_RESULTS_DISABLED;
		}

		// do callback.
		try {
			int Result = CallbackFunc(arg);

				// Return the result of the function.
			if (Result == 0) {
				return PUG_RESULTS_SUCCESS;
			}else {
				return PUG_RESULTS_FAILURE;
			}
		}
		catch (...) {
			return PUG_RESULTS_INVALID;
		}

		// Return the successful running.
		return PUG_RESULTS_SUCCESS;
	}

		// Enable and disable the callback.
	public: void Disable() {
		Disabled = true;
	}
	public: void Enable() {
		Disabled = false;
	}
	public: bool IsEnabled() {
		return !Disabled;
	}


		// Default constructor.
	public: PUG_CallBack() {

	}

		// Default destructor.
	public: ~PUG_CallBack() {

	}
};

	// The group of things that will get the callback.
struct PUG_CallBack_Group {
	
		// The ID for the group.
	int ID;

		// The name for the event listener.
	std::string Name;
	
		// The list of callbacks to trigger.
	std::vector<PUG_CallBack> Callbacks;

};

	// The engine that will keep track of the callbacks and migrate the calls back up if done correctly.
class PUG_CallBack_Engine {

		// The list of callbacks.
	private: std::vector<PUG_CallBack_Group> CallBacks;

		// The Mutex for the Callback groups.
	private: std::mutex mtx;

		// The global id for the group.
	private: int GroupID_Counter;


		// Try to lock the Mutex.
	private: void DoMutexLock() {
		#ifdef PUG_ENABLE_THREADING

			// Try to lock until it locks.
		for(;;){
			if (mtx.try_lock()) {				
				// It got the lock so return.
				return;
			}
		}
		#endif
	}

		// Unlock Mutex.
	private: void DoMutexUnlock() {
		#ifdef PUG_ENABLE_THREADING
		mtx.unlock();
		#endif
	}

		// Create a new group if it does not exist.
	private: int CreateNewGroup( std::string GroupName ) {

			// Find if the group exists.
		if (GetGroupIndex(GroupName) != -1) {
			return PUG_RESULTS_FAILURE;
		}
			
		PUG_CallBack_Group NewGroup;

			// Set the name.
		NewGroup.Name = GroupName;
			
			// Increase the counter and set it to in use.
		GroupID_Counter += 1;
		NewGroup.ID = GroupID_Counter;

			// Add the new group.
		CallBacks.push_back(NewGroup);

			// Return successful.
		return PUG_RESULTS_SUCCESS;
	}

		// Get the list of groups.
	public: std::vector<std::string> Info_GetGroupList() {

		std::vector<std::string> Results;

			// Lock the list so it wont change as we search it.
		DoMutexLock();

			// Get all of the names.
		for (unsigned int i = 0; i < CallBacks.size(); i++) {
			Results.push_back(CallBacks[i].Name);
		}

			// Do unlock.
		DoMutexUnlock();

		return Results;
	}

		// Get the list of call backs from a group.
	public: std::vector<PUG_CallBack> Info_GetCallbackFromGroup(std::string GroupName) {
		
			// Lock the list so it wont change as we search it.
		DoMutexLock();

			// Get the index.
		int Index = GetGroupIndex(GroupName);

			// If there is no group with that name, return nothing.
		if (Index == -1) {			
			DoMutexUnlock();

			std::vector<PUG_CallBack> Results;

			return Results;
		}

			// Make a local copy of it.
		PUG_CallBack_Group Tmp = CallBacks[Index];
	
			// Do unlock.
		DoMutexUnlock();

			// Return the callbacks.
		return Tmp.Callbacks;
	}

		// Remove the group.
	public: int DeleteGroup( std::string GroupName ) {

			// If threading is enabled force the mutex to be locked before hand.
		DoMutexLock();

		int IndexOfGroup = GetGroupIndex(GroupName);

			// If there is no group by that name.
		if (IndexOfGroup != -1) {

				// Unlock Mutex.
			DoMutexUnlock();

			return PUG_RESULTS_FAILURE;
		}

			// Remove the group and return success.
		CallBacks.erase(CallBacks.begin() + IndexOfGroup);

			// Unlock Mutex.
		DoMutexUnlock();

		return PUG_RESULTS_SUCCESS;
	}

		// Get the index for a group (mutex).
	private: int GetGroupIndex( std::string GroupName ) {
		for (unsigned int i = 0; i < CallBacks.size(); i++) {
			if (CallBacks[i].Name == GroupName) {
				return i;
			}
		}

		return -1;
	}
		
		// Add the callback to the event listener (do not use mutex inside this).
	public: int AddCallBack( std::string Name, std::string GroupName, int CallbackFunc(void *a) ) {

		PUG_CallBack NewCallback;

			// Fill in the callback.
		NewCallback.SetID(0); //todo
		NewCallback.SetName(Name);
		NewCallback.SetCallback(CallbackFunc);
		NewCallback.SetGroupID(0);//todo

			// If threading is enabled force the mutex to be locked before hand.
		DoMutexLock();

			// Find the group it goes in.
		int GroupIndex = GetGroupIndex(GroupName);

			// If the group index is not 0.
		if (GroupIndex != -1) {
			
				// Add the callback to the group.
			CallBacks[GroupIndex].Callbacks.push_back(NewCallback);

				// Unlock Mutex.
			DoMutexUnlock();

				// Return success.
			return PUG_RESULTS_SUCCESS;
		}		

			// It was not found so create a new group.
		if (CreateNewGroup(GroupName) == PUG_RESULTS_FAILURE) {

				// Unlock Mutex.
			DoMutexUnlock();

				// There was an error so return failure.
			return PUG_RESULTS_FAILURE;
		}else{

				// Find the group it goes in.
			int GroupIndex = GetGroupIndex(GroupName);

				// Add the callback to the group.
			CallBacks[GroupIndex].Callbacks.push_back(NewCallback);

				// Unlock Mutex.
			DoMutexUnlock();

				// Return success.
			return PUG_RESULTS_CREATEDNEW;
		}

			// Unlock the mutex.
		DoMutexUnlock();

			// Return that we created a new one.
		return PUG_RESULTS_CREATEDNEW;
	}

		// Trigger the callback.
	public: int DoCallBack( std::string GroupName, void *Value ) {

			// If threading is enabled force the mutex to be locked before hand.
		DoMutexLock();

			// Get the index.
		int GroupIndex = GetGroupIndex(GroupName);

			// Check if the index.
		if (GroupIndex == -1) {

				// Release the lock before quiting.
			DoMutexUnlock();

			return PUG_RESULTS_FAILURE;
		}

			// Make a local copy of the callbacks so we can unlock the mutex.
		PUG_CallBack_Group Tmp = CallBacks[GroupIndex];

			// Unlock Mutex.
		DoMutexUnlock();


			// Do the call backs for the group.
		for (unsigned int i = 0; i < Tmp.Callbacks.size(); i++) {
			Tmp.Callbacks[i].DoCallback(Value);
		}

		return PUG_RESULTS_SUCCESS;
	}

		// Display the list of groups and callbacks.
	public: void PUG_PS() {

			// Get the list of group names.
		std::vector<std::string> Names = Info_GetGroupList();

		for (unsigned int i = 0; i < Names.size(); i++) {

			std::string GroupName = "'" + Names[i] + "'";
			std::string Hooks = "  ID:  'Name'  IsDisabled\n";

				// Get the callbacks.
			std::vector<PUG_CallBack> Calls = Info_GetCallbackFromGroup(Names[i]);

			for (unsigned int g = 0; g < Calls.size(); g++) {

				Hooks += "  " + std::to_string(Calls[g].GetID()) + ": '" + Calls[g].GetName() + "' IsDisabled=";

				bool Disabled = Calls[g].IsEnabled();

				if (Disabled) {
					Hooks += "false\n";
				} else {
					Hooks += "true\n";
				}
			}

			std::cout << GroupName << std::endl << Hooks << std::endl;
		}
	}


		// Default constructor.
	public: PUG_CallBack_Engine() {	
		GroupID_Counter = 0;	
	}
};

	// The global variable that is defined for callbacks.
extern PUG_CallBack_Engine PUG;

#endif