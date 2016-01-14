//========================================================================
//
// OptionalContent.cc
//
// Copyright 2005 Michael Pfeiffer
//
//========================================================================

#include <alloc.h>
#include <memory.h>

#include "BeOptionalContent.h"

enum {
	kDefaultNumberOfIntentNames = 8
};

// Implementation of Bitvector
Bitvector::Bitvector(int initialSize)
	: size(initialSize)
	, allocatedSize(0)
	, vector(NULL)
{
	resize(initialSize);
}

Bitvector::~Bitvector() {
	free(vector);
	vector = 0;
	size = 0;
	allocatedSize = 0;
}

void Bitvector::set(int index) {
	resize(index + 1);
	vector[index / 8] |= (1 << (index % 8));
}

void Bitvector::set(int index, bool value) {
	if (value) {
		set(index);
	} else {
		clear(index);
	}
}

void Bitvector::clear(int index) {
	if (index < size) {
		vector[index / 8] &= ~(1 << (index % 8));
	}
}

bool Bitvector::isSet(int index) const {
	if (index < size) {
		return (vector[index / 8] & (1 << (index % 8))) != 0;
	}
	return false;
}

void Bitvector::clear() {
	int bytes = allocatedSize / 8;
	memset(vector, 0, bytes);
	size = 0;
}

void Bitvector::resize(int size) {
	if (this->size < size) {
		this->size = size;
	}

	if (allocatedSize < size) {
		// increase size of vector
		int bytes = (size + 7) / 8;
		int i = allocatedSize / 8;
		allocatedSize = bytes * 8;
		if (vector == NULL) {
			vector = (unsigned char*)malloc(allocatedSize);
		} else {
			vector = (unsigned char*)realloc(vector, allocatedSize);
		}
		// clear appended bits
		memset(vector + i, 0, (bytes - i));
	}
}


// Implementation of OCIntentName
OCIntentName::OCIntentName(GString* name, int id)
	: name(name)
	, id(id)
{
}

OCIntentName::~OCIntentName()
{
	delete name;
}

// Implementation of OCIntentNameRegistry
OCIntentNameRegistry::OCIntentNameRegistry()
{
	// initialize common intent names
	get("View");
	get("Design");
}

OCIntentNameRegistry::~OCIntentNameRegistry()
{
	for (int i = 0; i < intents.getLength(); i ++) {
		OCIntentName* name = (OCIntentName*)intents.get(i);
		delete name;
	}
}

OCIntentName * OCIntentNameRegistry::get(const char* name)
{
	for (int i = 0; i < intents.getLength(); i ++) {
		OCIntentName* intent = (OCIntentName*)intents.get(i);
		if (intent->getName()->cmp(name) == 0) {
			return intent;
		}
	}
	OCIntentName* intent = new OCIntentName(new GString(name), intents.getLength());
	intents.append(intent);
	return intent;
}

OCIntentName * OCIntentNameRegistry::get(int index) const
{
	if (index < intents.getLength()) {
		return (OCIntentName*)intents.get(index);
	}
	return NULL;
}

OCIntentName * OCIntentNameRegistry::getViewIntentName() {
	return get("View");
}

OCIntentName * OCIntentNameRegistry::getDesignIntentName() {
	return get("Design");
}

//
OCIntentNameIterator::OCIntentNameIterator(OCIntentNameRegistry* registry, Bitvector* bitvector)
	: index(0)
	, registry(registry)
	, bitvector(bitvector)
{
	forward();
}

void OCIntentNameIterator::forward() {
	while (index < bitvector->getSize() && !bitvector->isSet(index)) {
		index ++;
	}
}

OCIntentName* OCIntentNameIterator::next() {
	if (hasNext()) {
		OCIntentName* name = registry->get(index);
		forward();
		return name;
	}
	return NULL;
}


//
OCIntent::OCIntent()
	: intentNameIDs(kDefaultNumberOfIntentNames)
{
}

OCIntent::~OCIntent() {
}

bool OCIntent::init(Object* intent, OCIntentNameRegistry *registry) {
	if (intent == NULL) {
		// set default
		OCIntentName* intent = registry->getViewIntentName();
		intentNameIDs.set(intent->getId());
		return true;
	}

	// TODO
	return false;
}

OCIntentNameIterator OCIntent::iterator(OCIntentNameRegistry *registry) {
	return OCIntentNameIterator(registry, &intentNameIDs);
}
