

# Pugback
Pugback is a simple class that provides simple callbacks to c++. This is built to provide simple propagation of data for example user input. The user input can create a call back trigger for a group and any number of listeners can wait for it and process it.

![pageres](Pug.jpg.gif)

## General information
1. I use Mutex to make this thread safe, you can disable this by undefining 'PUG_ENABLE_THREADING'.
2. Callbacks for a group are called one at a time (a for loop). If a callback takes a long period of time to process the other callbacks will wait for it to finish before being called.
3. If a callback raises an exception it will just be ignored by the callback function.

## Usage
There is a global pug engine that is defined in the file that should be used. There is not a really good reason to define a pug_callback_engine for each callback unless you have a huge number of groups.

1. Add a hook with PUG.AddCallBack().
2. Call the hook with PUG.DoCallBack().



```c++
// The following is for adding a callback to a global function or static.

#include <iostream>
#include "Pugback.h"

int Example(void *A, void* context) {

	std::string Tmp = PugGetValue<std::string>(A);

	return 0;
}

int main() {

		// Create a callback "name", "group" and callback function.
	PUG.AddCallBack("Callback Name", "Example Group", Example);
	
	std::string Data = "Pugs"; // Example value to send.
	
	PUG.DoCallBack("Example Group", &Data); // Preform the callback.

	PUG.PUG_PS(); // Print out the list of groups and hooks.

	return 0;
}
```

For class members you have to provide context to the callback so it knows what class it's calling.
```c++
#include <iostream>
#include <string>
#include "Pugback.h"

class example {

	static int forwarder(void* value, void * context) {
		return static_cast<example*>(context)->Init(value, context);		
	}

	public: int Init(void *arg, void* context) {
	
		std::string Tmp = PugGetValue<std::string>(arg);

		return 0;
	}


	example() {
			// Add the hook.			
		PUG.AddCallBack("Callback Name", "Example group", &forwarder, this);
	
		std::string Test = "AAA";

			// Do the call back.
		PUG.DoCallBack("Example group", &Test);
	}

};
```
