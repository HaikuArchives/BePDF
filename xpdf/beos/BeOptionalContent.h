//========================================================================
//
// OptionalContent.h
//
// Copyright 2005 Michael Pfeiffer
//
//========================================================================

#ifndef OC_H
#define OC_H

#include "GList.h"
#include "Object.h"

// Bitvector that automatically grows on demand.
class Bitvector {
public:
	// Constructs the bitvector to the specified size.
	Bitvector(int initialSize);
	// Destructs the bitvector.
	virtual ~Bitvector();
	
	// Returns the current size.
	int getSize() const { return size; }
	
	// Sets the bit at index.
	// Grows the bitvector if required.
	void set(int index);
	
	// If value is true sets the bit at index
	// or otherwise clears the bit.
	// Grows the bitvector if required.
	void set(int index, bool value);
	
	// Clears the bit at index.
	void clear(int index);
	
	// Tests if the bit at index is set.
	// Returns false for index >= size.
	bool isSet(int index) const;
	
	// Clears the bitvector and sets its size to 0.
	void clear();

private:
	void resize(int size);

	int size; // in bits
	int allocatedSize; // in bits (is a multiple of 8)
	unsigned char* vector;
};

// OCIntentName repesents the name stored in an OCIntent.
class OCIntentName {

public:
	OCIntentName(GString* name, int id);
	virtual ~OCIntentName();

	GString* getName() const { return name; }
	int getId() const { return id; }

private:
	GString *name;
	int      id;
};

// Optional Content Intent Name Registry manages and owns all OCIntentNames.
class OCIntentNameRegistry {
public:
	OCIntentNameRegistry();
	virtual ~OCIntentNameRegistry();
	
	// Returns a OCIntentName; creates one on demand.
	OCIntentName *get(const char* name);
	// Returns the OCIntentName at specified index.
	// Returns NULL if index >= length.
	OCIntentName *get(int index) const;
	int getLength() const;

	// Returns the "View" intent name.
	OCIntentName *getViewIntentName();
	// Returns the "Design" intent name.
	OCIntentName *getDesignIntentName();

private:
	GList intents; // of type OCIntentName
};

class OCIntentNameIterator {
public:
	
	bool hasNext() const { return index < bitvector->getSize(); }
	OCIntentName* next();
	
private:
	// hide constructor
	OCIntentNameIterator(OCIntentNameRegistry* registry, Bitvector* bitvector);

	// for access to constructor
	friend class OCIntent;

	void forward();

	int index;
	const OCIntentNameRegistry* registry;
	const Bitvector* bitvector;
};

// Optional Content Intent.
class OCIntent {

public:
	
	OCIntent();
	virtual ~OCIntent();

	bool init(Object* intent, OCIntentNameRegistry *registry);

	bool contains(OCIntentName* intentName) const { return intentNameIDs.isSet(intentName->getId()); }
	OCIntentNameIterator iterator(OCIntentNameRegistry *registry);

private:
	Bitvector intentNameIDs;
};

#if 0
// TODO Optional Content Usage Application
class OCUsageApplication {

public:
	OCUsageApplication();
	virtual ~OCUsageApplication();

private:
	OCIntentName *name; // TODO check if type is correct (GString* instead?)
	OCGroups *groups;
	// TODO category
};

// TODO Optional Content Usage
class OCUsage {

public:
	OCUsage();
	virtual ~OCUsage();

private:
	// TODO creatorInfo
	// TODO language
	// TODO export
	// TODO zoom
	// TODO print
	// TODO view
	// TODO user
	// TODO pageElement
	
};

// Optional Content Object Types
enum OCObjectType {
	OCGroupType,
	OCMembershipType
};

// Optional Content Object the super class for OCGroup and OCMembership
class OCObject {
public:
	OCObject();
	virtual ~OCObject();

	virtual OCObjectType getType() = 0;
	bool isOCGroup()      { return getType() == OCGroupType; }
	bool isOCMembership() { return getType() == OCMembershipType; }
};

// Optional Content Group
class OCGroup : public OCObject {

public:
	// Constructs this OCGroup.
	OCGroup(XRef *xref, Ref ref); 
	virtual ~OCGroup();
	
	OCObjectType getType() { return OCGroupType; }
	
	// Frees allocated memory.
	virtual ~OCGroup();
	
	// Returns true if constructor was successful.
	bool IsValid();
	
	// Returns the state of this OCGroup.
	bool getState() { return state; }

	// Sets the state of this OCGroup.
	void setState(bool state) { this->state = state; }

	// Returns the Ref of this OCGroup.
	Ref getRef() { return ref; }

	// Returns the name of this OCGroup.
	GString *getName() { return name; }
	
	// Returns the intent.
	OCIntent *getIntent() { return &intent; }
	
	OCUsage *getUsage() { return &usage; }

private:
	bool     state;
	Ref      ref;
	Dict    *dict;
	GString *name;
	OCIntent intent;
	OCUsage  usage;
};

// Collection of OCGroup objects.
class OCGroups {
public:
	// Constructs this OCGroups object.
	// If ownsOCGs is true the constructor of this collection frees the OCGroup
	OCGroups(bool ownsOCGs = false);
	// Destructor.
	virtual ~OCGroup();

	// Returns the number of OCGs.
	int getLength() { groups.getLength(); }
	
	// Appends this group if a group with the same ref 
	// does not already exist.
	// Assumes group is not NULL.
	bool append(OCGroup *group);
	
	// Returns the OCGroup at the specified index.
    // Assumes 0 <= i < length.
	OCGroup* get(int index);
	
	// Returns the OCGroup with the specified ref.
	// Returns NULL if not found.
	OCGroup* get(Ref ref);

private:
	GList groups;
};

// Optional Content Membership
class OCMembership : public OCObject {

public:
	enum VisibilityPolicy {
		AllOn,
		AnyOn,
		AnyOff,
		AllOff
	};

	OCMembership(XRef *xref, Ref ref, OCGroups* groups);
	virtual ~OCMembership();

	OCObjectType getType() { return OCMembershipType; }

	bool isValid();
	
	Ref getRef();

	void determineState();

	// returns the state of this membership dictionary
	bool getState();
	
private:

	bool     state;
	Ref      ref;
	OCGroups *groups;
	Dict     *dict;
	// TODO VE (visibility expression)
};

enum OCBaseState {
	On,
	Off,
	Unchanged
};

enum OCListMode {
	AllPages,
	VisiblePages
};

// Optional Content Configuration
class OCConfiguration {

public:
	OCConfiguration();
	virtual ~OCConfiguration();

private:
	GString *name;
	GString *creator;
	OCBaseState baseState;
	OCGroups on;
	OCGroups off;
	OCIntent intent;
	// TODO AS
	// TODO order
	OCListMode listMode;
	// TODO radioButtonGroups
	OCGroups locked;	
};


// Optional Content Properties
class OCProperties {

public:
	OCProperties();
	virtual ~OCProperties();

	// Returns all OCGroups.
	OCGroups* getGroups();
	
	// Returns the default OCConfiguration.
	OCConfiguration *getDefault();
	
	// Returns the number of OCConfigurations.
	int getLength();
	
	// Returns the OCConfiguration at index.
	// Assumes 0 <= index < length
	OCConfiguration *getConfiguration(int index);

private:
	OCGroups *groups;
	OCConfiguration *defaultConfiguration;
	GList configurations; // of type OCConfiguration
};
#endif

#endif // OC

