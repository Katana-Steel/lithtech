
#if !defined(FINITESTATEMACHINE_H)
#define FINITESTATEMACHINE_H

#pragma warning( disable : 4786 )
#include <map>
#include <stack>
#include <assert.h>

#ifndef CALLBACK_H
#include "Callback.h"
#endif

#include "iltmessage.h"



template< class StateType, class MessageType >
class FiniteStateMachine
{
private :

	struct StateData;
	typedef std::map<StateType,StateData>    StateMap;
	typedef std::map<MessageType, StateType> TransitionMap;
	typedef std::map<StateType, StateType> AutomaticMap;
	typedef std::deque<MessageType> MessageBuffer;

	enum MicrosoftHack
	{
		max_recursion = 100
	};

public :

	typedef Callback1v<FiniteStateMachine*> CallbackFunction;

	typedef Callback2v<const FiniteStateMachine &, StateType>   ChangingStateCallbackFunction;
	typedef Callback2v<const FiniteStateMachine &, MessageType> ProcessingMessageCallbackFunction;
	typedef Callback2v<const FiniteStateMachine &, StateType>   NextStateCallbackFunction;

	typedef MessageBuffer::const_iterator const_MessageIterator;

#ifdef _WIN32
	friend ILTMessage & operator<<(ILTMessage & out, const FiniteStateMachine & val);
	friend ILTMessage & operator>>(ILTMessage & in, FiniteStateMachine & val);
#else
	friend ILTMessage & operator<< <>(ILTMessage & out, const FiniteStateMachine & val);
	friend ILTMessage & operator>> <>(ILTMessage & in, FiniteStateMachine & val);
#endif

private :

	struct StateData
	{
		StateType id;

		CallbackFunction updateCallback;
		CallbackFunction initCallback;
		CallbackFunction termCallback;

		TransitionMap transitions;

		bool       hasAutomatic;
		StateType  automatic;

		StateData( StateType new_id, 
				   CallbackFunction new_update,
				   CallbackFunction new_init,
				   CallbackFunction new_term )
			: id(new_id),
			  updateCallback(new_update),
			  initCallback(new_init),
			  termCallback(new_term),
		      hasAutomatic(false),
		      automatic(StateType(0)) { }

		StateData()
			: id( StateType(0) ),
			  updateCallback( CallbackFunction() ),
			  initCallback( CallbackFunction() ),
			  termCallback( CallbackFunction() ),
			  hasAutomatic(false),
			  automatic(StateType(0)) { }

	};


public :
	// Utilities for the state callbacks.
	template< class Client, class Member>
	static CallbackFunction
	MakeCallback(Client & client, Member member)
	{
		return make_callback1v<FiniteStateMachine*>(client,member);
	}

	template< class Function >
	static CallbackFunction
	MakeCallback(Function function)
	{
		return make_callback1v<FiniteStateMachine*>(function);
	}

	static CallbackFunction
	MakeCallback()
	{
		return CallbackFunction();
	}

	// Utilities for the ChangingState and ProcessingMessage callbacks.
	template< class Client, class Member>
	static ChangingStateCallbackFunction
	MakeChangingStateCallback(Client & client, Member member)
	{
		return make_callback2v<const FiniteStateMachine &, StateType>(client,member);
	}

	template< class Function >
	static ChangingStateCallbackFunction
	MakeChangingStateCallback(Function function)
	{
		return make_callback2v<const FiniteStateMachine &, StateType>(function);
	}

	static ChangingStateCallbackFunction
	MakeChangingStateCallback()
	{
		return ChangingStateCallbackFunction();
	}

	template< class Client, class Member>
	static ProcessingMessageCallbackFunction
	MakeProcessingMessageCallback(Client & client, Member member)
	{
		return make_callback2v<const FiniteStateMachine &, MessageType>(client,member);
	}

	template< class Function >
	static ProcessingMessageCallbackFunction
	MakeProcessingMessageCallback(Function function)
	{
		return make_callback2v<const FiniteStateMachine &, MessageType>(function);
	}

	static ProcessingMessageCallbackFunction
	MakeProcessingMessageCallback()
	{
		return ProcessingMessageCallbackFunction();
	}

	template< class Client, class Member>
	static NextStateCallbackFunction
	MakeNextStateCallback(Client & client, Member member)
	{
		return make_callback2v<const FiniteStateMachine &, StateType>(client,member);
	}

	template< class Function >
	static NextStateCallbackFunction
	MakeNextStateCallback(Function function)
	{
		return make_callback2v<const FiniteStateMachine &, StateType>(function);
	}

	static NextStateCallbackFunction
	MakeNextStateCallback()
	{
		return NextStateCallbackFunction();
	}

public:
	
	FiniteStateMachine()
		: currentStatePtr(0),
		  hasNextState(false) { }
		

	// Whenever a state is entered, init_cb will be called.  On each update update_cb will be called.  Just
	// before a state change is made term_cb is called.  It will be followed by the new state's init_cb.
	void DefineState( StateType id, CallbackFunction update_cb = CallbackFunction(), CallbackFunction init_cb = CallbackFunction(), CallbackFunction term_cb = CallbackFunction());

	// State-Message-State Transition:
	//   When message is processed during the Update(), if in original_state, a transition will be made to a new 
	//   state.  A transition is a call to original_state's term_cb. Then, if final_state has 
	//   an init_cb, it will be called.  This will be repeated for each message in the message queue.
	//   When the queue is empty the final state's update_cb will be called.
	void DefineTransition( StateType original_state, MessageType message, StateType final_state);

	// Message-State Transition (default transition):
	//   When a message is processed, if original_state did not have a transition defined for the
	//   message, a transition will be made to final_state instead.  This is a default transition.
	//   If a State-Message-State transition has higher precedence than a Message-State transition.
	void DefineTransition( MessageType message, StateType final_state );

	// State-State transition (automatic transition):
	//   When a transition to original_state is made, the original_state's init_cb will be called.
	//   Then a transition will be made to final_state,  calling original_state's term_cb and calling
	//   final_state's init_cb.  If final_state also has an automatic transition, the same thing
	//   will be repeated.  Otherwise the message queue will continue to be processed and, if no other
	//   state transitions occur, final_state's update_cb will be called and the fsm will be left in
	//   final_state.  Note that original_state will never call its update_cb.
	void DefineTransition( StateType original_state, StateType final_state );

	// This forces a new state.  The current state will have its term_cb called, and the
	// new state will have its init_cb called.  If successful, the message buffer is emptied and NextState is reset!
	void SetState( StateType id );

	// This declares the initial state for the state machine.
	// If it does not have a state at the time this function is called, 
	// it will be set to this state.  init_cb is _not_ called!
	void SetInitialState( StateType id );

	// This state will be switched to immediately in the next update.  Another call to SetNextState will
	// replace the previous next state.  At update, The current state's term_cb will be 
	// called. The new state's init_cb will be called.  All messages in the buffer will
	// be interpreted as if this new state is the current state.
	void SetNextState( StateType id );

	// Returns true if a next state has be set with SetNextState since the last update.
	bool HasNextState() const;

	// Push a message onto the message buffer.
	void AddMessage( MessageType msg );

	// Remove all messages from the message buffer.
	void ClearMessages();

	// Processes message buffer until it is empty.  Each state transition
	// will call old state's term_cb and the new state's init_cb.  After the message buffer
	// is empty, the final state's update_cb will be called.  If, after that, there are more
	// messages in the message buffer, then those messages will be processed and if state transition
	// is made the new state's update_cb will be called. And again, the messages will be processed.
	// Note that an infinite loop will occur if update_cb always sends a message which does not cause
	// a state transition.
	void Update();

	// Accessor functions.
	bool IsValid(StateType state) const { return states.find(state) != states.end(); }
	StateType GetState() const { assert( currentStatePtr ); return currentStatePtr ? currentStatePtr->id : StateType(0); }
	
	const_MessageIterator BeginMessages() const { return messages.begin(); }
	const_MessageIterator EndMessages() const   { return messages.end(); }

	bool HasAutomatic( StateType state ) const;
	StateType GetAutomatic( StateType state ) const;

	bool HasDefault( MessageType message ) const { return defaults.find(message) != defaults.end(); }
	StateType GetDefault( MessageType message ) const;

	bool HasTransition( StateType original_state, MessageType message ) const;
	StateType GetTransition( StateType original_state, MessageType message ) const;


	// changing_state_cb will be called just before a state change occurs.  It is called
	// just after current state's term_cb and just before current state is changed
	// to the new state.
	void SetChangingStateCallback( ChangingStateCallbackFunction changing_state_cb ) { changingStateCallback = changing_state_cb; }

	// processing_message_cb is called just after the message to process is popped off the message
	// buffer but before anything is done with it.
	void SetProcessingMessageCallback( ProcessingMessageCallbackFunction processing_message_cb) { processingMessageCallback = processing_message_cb; }

	// next_state_cb will be called whenever SetNextState is used to set the next state.
	void SetNextStateCallback( NextStateCallbackFunction next_state_cb ) { nextStateCallback = next_state_cb; }

private :

	void RecursiveUpdate(int recursion_level);

	StateMap states;
	TransitionMap defaults;
	AutomaticMap  automatics;

	StateData * currentStatePtr;

	MessageBuffer messages;

	bool      hasNextState;
	StateType nextStateId;

	ChangingStateCallbackFunction     changingStateCallback;
	ProcessingMessageCallbackFunction processingMessageCallback;
	NextStateCallbackFunction         nextStateCallback;
};




// When MS supports the export keyword, all below here can be moved into fsm.cpp

template< class StateType, class MessageType >
void FiniteStateMachine<StateType,MessageType>::DefineState( StateType id, CallbackFunction update_cb, CallbackFunction init_cb, CallbackFunction term_cb )
{
	states[id] = StateData(id,update_cb,init_cb,term_cb);
}


template< class StateType, class MessageType >
void FiniteStateMachine<StateType,MessageType>::DefineTransition( StateType original_state, MessageType message, StateType final_state)
{
	if( !IsValid(original_state) || !IsValid(final_state) )
	{
		assert(0);
		return;
	}

	states[original_state].transitions[message] = final_state;
}

template< class StateType, class MessageType >
void FiniteStateMachine<StateType,MessageType>::DefineTransition( MessageType message, StateType final_state)
{
	if( !IsValid(final_state) )
	{
		assert(0);
		return;
	}

	defaults[message] = final_state;
}

template< class StateType, class MessageType >
void FiniteStateMachine<StateType,MessageType>::DefineTransition( StateType original_state, StateType final_state)
{
	if( !IsValid(original_state) || !IsValid(final_state) )
	{
		assert(0);
		return;
	}

	states[original_state].hasAutomatic = true;
	states[original_state].automatic = final_state;

	assert( states[original_state].updateCallback.isNil() );
}

template< class StateType, class MessageType >
void FiniteStateMachine<StateType,MessageType>::SetState( StateType id )
{
	if( IsValid(id) )
	{
		ClearMessages();
		hasNextState = false;

		if( currentStatePtr ) currentStatePtr->termCallback(this);

		if( !changingStateCallback.isNil() ) changingStateCallback(*this,id);

		currentStatePtr = &states[id];
		currentStatePtr->initCallback(this);
	}
	else
	{
		assert(0);
		if( currentStatePtr ) currentStatePtr->termCallback(this);
		currentStatePtr = 0;
	}
}

template< class StateType, class MessageType >
void FiniteStateMachine<StateType,MessageType>::SetInitialState( StateType id )
{
	if( !currentStatePtr )
	{
		if( IsValid(id) )
		{
			currentStatePtr = &states[id];
		}
		else
		{
			assert(0);
		}
	}
}

template< class StateType, class MessageType >
void FiniteStateMachine<StateType,MessageType>::SetNextState( StateType id )
{
	if( !nextStateCallback.isNil() ) nextStateCallback(*this,id);

	nextStateId = id;
	hasNextState = true;
}

template< class StateType, class MessageType >
bool FiniteStateMachine<StateType,MessageType>::HasNextState() const
{
	return hasNextState;
}

template< class StateType, class MessageType >
void FiniteStateMachine<StateType,MessageType>::AddMessage( MessageType msg ) 
{ 
	messages.push_back(msg);
}

template< class StateType, class MessageType >
void FiniteStateMachine<StateType,MessageType>::ClearMessages() 
{ 
	messages.clear();
}

template< class StateType, class MessageType >
void FiniteStateMachine<StateType,MessageType>::Update() 
{
	// Do the transition set-up by SetNextState.
	if( hasNextState )
	{

		if( IsValid(nextStateId) )
		{
			if( currentStatePtr ) currentStatePtr->termCallback(this);

			if( !changingStateCallback.isNil() ) changingStateCallback(*this,nextStateId);

			// This line ensures that a call to HasNextState
			// will return false inside the new state's initCallback.
			// If you don't want this behaviour, just remove
			// this line.
			hasNextState = false;

			currentStatePtr = &states[nextStateId];
			currentStatePtr->initCallback(this);
		}
		else
		{
			// If we have an invalid next state, just stay in the current state.
			// But this shouldn't be happening!  All states should be valid!

			assert(0);
		}

		hasNextState = false;
	}

	// Handle all messages and updates of the current state.
	RecursiveUpdate(0);
}

template< class StateType, class MessageType >
void FiniteStateMachine<StateType,MessageType>::RecursiveUpdate(int recursion_level) 
{
	// Be sure we don't go into an infinite loop.
	assert( recursion_level < max_recursion );
	if( recursion_level >= max_recursion )
		return;

	// handle messages.  
	StateData * new_state_ptr = 0;
	while( !messages.empty() )
	{
		const MessageType current_msg = messages.front();
		messages.pop_front();

		if( !processingMessageCallback.isNil() ) processingMessageCallback(*this, current_msg);

		// Try to find a state transition for this message in the state data.
		// If that fails, check our default transitions.
		// If that fails, new_state_ptr will stay null and we'll continue down the message stack.
		if( currentStatePtr
			&& currentStatePtr->transitions.find( current_msg ) != currentStatePtr->transitions.end() )
		{
			new_state_ptr = &states[ currentStatePtr->transitions[current_msg] ];
		}
		else if( defaults.find( current_msg ) != defaults.end() )
		{
			new_state_ptr = &states[ defaults[ current_msg ] ];
		}
		
		if( new_state_ptr )
		{

			if( currentStatePtr ) currentStatePtr->termCallback(this);

			if( !changingStateCallback.isNil() ) changingStateCallback(*this, new_state_ptr->id );

			currentStatePtr = new_state_ptr;
			if( !currentStatePtr->initCallback.isNil() )
			{
				currentStatePtr->initCallback(this);
			}

			// handle a state with an automatic transition
			while( currentStatePtr->hasAutomatic )
			{
				currentStatePtr->termCallback(this);

				if( !changingStateCallback.isNil() ) changingStateCallback(*this, currentStatePtr->automatic );

				new_state_ptr = &states[ currentStatePtr->automatic ];
				currentStatePtr = new_state_ptr;

				if( !currentStatePtr->initCallback.isNil() )
				{
					currentStatePtr->initCallback(this);
				}
			}
		}
	}
	
	// Call update and make an automatic transition if needed.
	if( currentStatePtr && (new_state_ptr || 0 == recursion_level ) )
	{
		currentStatePtr->updateCallback(this);
	}

	if( !messages.empty() )
		RecursiveUpdate(recursion_level+1);
}


template< class StateType, class MessageType >
bool FiniteStateMachine<StateType,MessageType>::HasAutomatic( StateType state_id )  const
{
	if( IsValid(state_id) ) 
	{
		StateMap::const_iterator iter = states.find(state_id);
		if( iter != states.end() )
		{
			const StateData & original_data = iter->second;
		
			return original_data.hasAutomatic;
		}
	}

	return false;
}

template< class StateType, class MessageType >
StateType FiniteStateMachine<StateType,MessageType>::GetAutomatic( StateType state_id ) const
{
	if( IsValid(state_id) ) 
	{
		StateMap::const_iterator iter = states.find(state_id);
		if( iter != states.end() )
		{
			const StateData & original_data = iter->second;
	
			return original_data.automatic;
		}
	}

	return StateType(0);
}



template< class StateType, class MessageType >
bool FiniteStateMachine<StateType,MessageType>::HasTransition( StateType original_state, MessageType message ) const
{
	if( IsValid(original_state) )
	{
		StateMap::const_iterator iter = states.find(original_state);
		if( iter != states.end() )
		{
			const StateData & original_data = iter->second;
			if( original_data.hasAutomatic )
			{
				return true;
			}
			else if( original_data.transitions.find(message) != original_data.transitions.end() )
			{
				return true;
			}
		}

		if( HasDefault(message) )
		{
			return true;
		}
	}

	return false;
}

template< class StateType, class MessageType >
StateType FiniteStateMachine<StateType,MessageType>::GetTransition( StateType original_state, MessageType message ) const
{
	if( IsValid(original_state) )
	{
		StateMap::const_iterator iter = states.find(original_state);
		if( iter != states.end() )
		{
			const StateData & original_data = iter->second;

			if( original_data.hasAutomatic )
			{
				return original_data.automatic;
			}

			TransitionMap::const_iterator transition_iter = original_data.transitions.find(message);
			if( transition_iter != original_data.transitions.end() )
			{
				return transition_iter->second;
			}
			else if( HasDefault(message) )
			{
				return GetDefault(message);
			}
		}
	}

	return StateType(0);
}


template< class StateType, class MessageType >
StateType FiniteStateMachine<StateType,MessageType>::GetDefault( MessageType message ) const
{
	if( HasDefault(message) )
	{
		TransitionMap::const_iterator iter = defaults.find(message);
		if( iter != defaults.end() )
		{
			return iter->second;
		}
	}

	return StateType(0);
}

template< class StateType, class MessageType >
LMessage & operator<<(LMessage & out, const FiniteStateMachine<StateType,MessageType> & val)
{
	typedef FiniteStateMachine<StateType,MessageType>::MessageBuffer MessageBuffer;

	// Save our state.
	out.WriteByte( val.currentStatePtr != 0 );
	if( val.currentStatePtr )
		out.WriteDWord(val.GetState());

	// Save our message buffer.
	out.WriteDWord(val.messages.size());
	for( MessageBuffer::const_iterator iter = val.messages.begin();
		 iter != val.messages.end(); ++iter )
	{
		out.WriteDWord(*iter);
	}

	// Save our next state info.
	out.WriteByte( val.hasNextState );
	out.WriteDWord( val.nextStateId );

	return out;
}

template< class StateType, class MessageType >
LMessage & operator>>(LMessage & in, FiniteStateMachine<StateType,MessageType> & val)
{
	// Load our state.
	const bool set_current_state_ptr = ( in.ReadByte() == LTTRUE );

	val.currentStatePtr = 0;
	if( set_current_state_ptr )
	{
		StateType id = StateType(in.ReadDWord());
		if( val.states.find(id) != val.states.end() )
		{
			val.currentStatePtr = &val.states[id];
		}
		else
		{
			ASSERT( 0 );
		}
	}


	// Load our message buffer.
	val.ClearMessages();
	const int size = in.ReadDWord();
	for( int i = 0; i < size; ++i )
	{
		val.messages.push_back(MessageType(in.ReadDWord()));
	}

	val.hasNextState = (in.ReadByte() == LTTRUE );
	val.nextStateId  = StateType(in.ReadDWord());

	return in;
}



#endif //FINITESTATEMACHINE_H
