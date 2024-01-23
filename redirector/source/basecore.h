#ifndef REDIRECTOR_BASE_CORE_H_
#define REDIRECTOR_BASE_CORE_H_

class AbstractComponent;

// Abstract application core class. 
// Must follow the "mediator" pattern.
class AbstractCore
{
public:
	// Event types.
	enum Event
	{
		UpdateConfig,	// Application config update.
		StopEvent			// Application stop event.
	};
	
	// Default destructor.
	virtual ~AbstractCore() = default;

	// Wait for termination.
	virtual void Wait() = 0;

	// Notifying components of an event.
	// @param component - sender component.
	// @param event - event type.
	virtual void Notify(AbstractComponent* component, Event event) = 0;
};

// Abstract core component class. 
// Must follow the "mediator" pattern.
class AbstractComponent
{
protected:
	AbstractCore* m_Mediator;

public:
	AbstractComponent(AbstractCore* mediator) :
		m_Mediator{ mediator }
	{ }

	virtual ~AbstractComponent() = default;

	void SetMediator(AbstractCore* mediator) {
		m_Mediator = mediator;
	}
};

#endif // !REDIRECTOR_BASE_CORE_H_
