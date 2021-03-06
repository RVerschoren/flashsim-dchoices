/* Copyright 2009, 2010 Brendan Tauras */

/* ssd_ftl.cpp is part of FlashSim. */

/* FlashSim is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version. */

/* FlashSim is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details. */

/* You should have received a copy of the GNU General Public License
 * along with FlashSim.  If not, see <http://www.gnu.org/licenses/>. */

/****************************************************************************/

/* Ftl class
 * Brendan Tauras 2009-11-04
 *
 * This class is a stub class for the user to use as a template for implementing
 * his/her FTL scheme.  A few functions to gather information from lower-level
 * hardware are added to assist writing a FTL scheme.  The Ftl class should
 * rely on the Garbage_collector and Wear_leveler classes for modularity and
 * simplicity. */

#include <new>
#include <assert.h>
#include <stdio.h>
#include "ssd.h"
#include <iostream>

using namespace ssd;

Ftl::Ftl(Controller &controller):
	controller(controller),
	//garbage(gca),
	wear(*this)
{
	
	///@TODO Implement and enable remaining GCAs
	switch (GC_ALGORITHM)
	{
	//case 0:
	//	gc_impl = new GC_FIFO();
	//	break;
	//case 1:
	//	gc_impl = new GCImpl_Greedy(*this);
	//	break;
	case 2:
		garbage = new GCImpl_Random(this);
		std::cout << "RANDOM" << std::endl;
		break;
	case 3:
		garbage = new GCImpl_DChoices(this);
		std::cout << "DCHOICES"<< DCHOICES_D << std::endl;
		break;
	default:
		garbage = new GCImpl_Random(this);
		std::cout << "GREEDY" << std::endl;
	}
	return;
}

Ftl::~Ftl(void)
{
	if(garbage != nullptr)
	{
		delete garbage;
		garbage = nullptr;
	}
	return;
}

void Ftl::initialize()
{
	return;
}

enum status Ftl::read(Event &event)
{
	return SUCCESS;
}

enum status Ftl::write(Event &event)
{
	return SUCCESS;
}

enum status Ftl::erase(Event &event)
{
	return SUCCESS;
}

enum status Ftl::merge(Event &event)
{
	return SUCCESS;
}

void Ftl::garbage_collect(Event &event)
{
	return;
}

ssd::ulong Ftl::get_erases_remaining(const Address &address) const
{
	return controller.get_erases_remaining(address);
}

void Ftl::get_least_worn(Address &address) const
{
	controller.get_least_worn(address);
	return;
}

enum page_state Ftl::get_state(const Address &address) const
{
	return controller.get_state(address);
}

ssd::Page* Ftl::get_page(const Address &addr) const
{
	return controller.get_page(addr);
}

ssd::Block* Ftl::get_block(const Address &addr) const
{
	return controller.get_block(addr);
}

ssd::Plane* Ftl::get_plane(const Address &addr) const
{
	return controller.get_plane(addr);
}