
/************************************************************************
 * 									*
 * Copyright (C) 2004  Christina Warrender				*
 * 									*
 * This file is part of CyCells.					*
 *									*
 * CyCells is free software; you can redistribute it and/or modify it 	*
 * under the terms of the GNU General Public License as published by	*
 * the Free Software Foundation; either version 2 of the License, or	*
 * (at your option) any later version.					*
 *									*
 * CyCells is distributed in the hope that it will be useful,		*
 * but WITHOUT ANY WARRANTY; without even the implied warranty of	*
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the	*
 * GNU General Public License for more details.				*
 *									*
 * You should have received a copy of the GNU General Public License	*
 * along with CyCells; if not, write to the Free Software Foundation, 	*
 * Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA	*
 *									*
 ************************************************************************/
/************************************************************************
 * file cellType.h                                                      *
 * Declarations for CellType                                            * 
 * Attributes and behavior for all cells of one type                    *
 ************************************************************************/

#ifndef CELLTYPE_H
#define CELLTYPE_H

#include <string>
#include <vector>
#include "cell.h" 

using namespace std;

class Sense;
class Process;
class Cond;
class Action;

class CellType {
  public:
    enum Dist { FIXED, UNIFORM, GAUSSIAN, LOGNORMAL };

    //--------------------------- CREATORS --------------------------------- 
    explicit CellType(const string type_name);
    CellType(const string type_name, double cell_radius);
    // copy constructor not used
    ~CellType();			

    //------------------------- MANIPULATORS -------------------------------
    // assignment not used

    // routines to add parameterized activities
    void addAttribute(string name, Dist initFlag, double init1, double init2,
				   Dist randFlag, double rand1, double rand2)
	{attributes.push_back(Attribute(name, initFlag, init1, init2, 
				   randFlag, rand1, rand2));};
    void addActivity(Cond *pc, Action *pa)
	{activities.push_back(Activity(pc, pa));};
    void addAction(Action *pa) {unconditionals.push_back(pa);};
    void addProcess(Process *pp) {processes.push_back(pp);};
    void addSense(Sense *ps) {sensors.push_back(ps);};

    // routines to set various parameters
    void setRadius(double r) {m_radius = r;};
    void setSpeed(double s) {m_speed = s;};

    // the following routines actually do not change the CellType object,
    // but individual Cell objects that have this CellType

    // cell-specific initialization for this type of cell;
    // sets number of internal variables and their initial values
    void initializeCell(Cell *pc);	// for new daughter cells
    void randomizeCell(Cell *pc);	// for random initialization/injection

    // effect of environment on current cell 
    void sense(Cell *pc, double deltaT);

    // internal processing on current cell
    void process(Cell *pc, double deltaT);

    // carry out each of this cell type's activities on current cell
    // some of these activities actually modify other cells or molecular fields
    void act(Cell *pc, double deltaT);

    // carry out sense, process and act for one cell for one timestep
    // replaces both sense and process
    void update(Cell *pc, double deltaT);

    //--------------------------- ACCESSORS --------------------------------
    const string& getName() const {return m_name;};
    double getRadius() const {return m_radius;};
    double getSpeed() const {return m_speed;};
    int getNumAttributes() const {return attributes.size();};
    int getAttributeIndex(const string& attrName) const;
    const string getAttributeName(int index) const 
      {return attributes[index].m_name;};

    bool isMatch(const string& type_name) const;

  private:
    const string m_name;	// for user reference
    double m_radius;
    double m_speed;		// unimpeded speed of this cell type       

    struct Attribute		// represents internal Cell data
    {
      string m_name;
      Dist m_initFlag;
      double m_init1, m_init2;			// used by initializeCell 
      Dist m_randFlag;
      double m_rand1, m_rand2;			// used by randomizeCell
      Attribute(string n, Dist initFlag, double init1, double init2, 
			  Dist randFlag, double rand1, double rand2) : 
	m_name(n), m_initFlag(initFlag), m_init1(init1), m_init2(init2),
	m_randFlag(randFlag), m_rand1(rand1), m_rand2(rand2) {};
    };

    struct Activity
    {
      Cond *condition;
      Action *action;
      Activity(Cond *pc, Action *pa) : 
	condition(pc), action(pa) {};
    };

    vector <Attribute> attributes;	
    vector<Sense*> sensors; 		// modify internal state according to
					// external environment
    vector<Process*> processes;		// internal processing functions
    vector<Action*> unconditionals;	// unconditional Actions
    vector<Activity> activities; 	// condition-action pairs           

    // not used
    CellType(const CellType &rct);
    CellType operator = (const CellType &rct);

  friend ostream& operator<<(ostream& outfile, const CellType& t);    
};

#endif

