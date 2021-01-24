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
 * file simView3D.cc                                                    * 
 * render() routine for SimView3D                                       *    
 * Allows 3D display of tissue model		                        * 
 ************************************************************************/
                                                                        
#include "simView3D.h"
#include <GL/gl.h>  
#include "tissue.h"
#include "cellType.h"

#include <iostream>
using namespace std;

/************************************************************************ 
 * SimView3D()                                                          *
 *   Constructor                                                        *
 *                                                                      *
 * Parameters                                                           *
 *   wxWindow *parent:          window containing this view             *
 *   int x, y, w, h:  		position (x,y) and size (w,h) of view   *
 *   const Tissue *t;		pointer to model tissue                 *
 *                                                                      *
 * Returns - nothing                                                    *
 ************************************************************************/
SimView3D::SimView3D(wxWindow *parent, int x, int y, int w, int h, 
	const Tissue *t) 
	: SimView(parent, x, y, w, h), 
	  m_tissuep(t), m_first(true)
{
  // get tissue size info (do once at beginning rather than repeatedly)
  m_xsize = m_tissuep->getXSize();
  m_ysize = m_tissuep->getYSize();
  m_zsize = m_tissuep->getZSize();
//  m_gridsize = m_tissuep->getGridSize();
//  m_xnum = m_xsize/m_gridsize;
//  m_ynum = m_ysize/m_gridsize;
//  m_znum = m_zsize/m_gridsize;

  // get some info about types of cells & molecules to display
  int numCellTypes = m_tissuep->getNumCellTypes();
  m_cellSizes = new double[numCellTypes];
  for (int i=0; i<numCellTypes; i++)
    m_cellSizes[i] = m_tissuep->getCellType(i)->getRadius();

  // for drawing cells as spheres
  quadratic=gluNewQuadric();  
  gluQuadricNormals(quadratic, GLU_SMOOTH);   // Create Smooth Normals
  gluQuadricTexture(quadratic, GL_TRUE);      // Create Texture Coords ( NEW )
}

/************************************************************************ 
 * render()                                                             *
 *   OpenGL code to draw current view of model                          *
 *                                                                      *
 * Parameters -                                                         *
 *   wxPaintDC& dc:  	device context for drawing; not used here       *
 *                                                                      *
 * Returns - nothing                                                    *
 ************************************************************************/
void SimView3D::render(wxPaintDC& dc)
{
  SetCurrent();

  if (m_first)
  {
    // set up lighting 
    GLfloat light_position[] = { 1.0, 1.0, 1.0, 0.0 };
    GLfloat white_light[] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat lmodel_ambient[] = { 0.3, 0.3, 0.3, 0.3 };
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, white_light);
    glLightfv(GL_LIGHT0, GL_SPECULAR, white_light);
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    glClearColor(1.0, 1.0, 1.0, 0.0);	// set black background; no blending
    glShadeModel(GL_SMOOTH);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);

    // set size/projection - should be in resize routine?
    glViewport(0, 0, width, height);	
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-10, 10+m_xsize, -10, 10+m_ysize, -10, 10+m_zsize);

    m_first = false;
  }

  // clear; get ready to do model/view transformations
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  // shift 'camera' back to see positive z axis
  glTranslatef(0.0, 0.0, -10-m_tissuep->getZSize());

  renderCells();

//  renderMolecules();   

  glFlush();
  SwapBuffers();
}

/************************************************************************ 
 * renderCells()                                                        *
 *   OpenGL code to draw visible cells in model                         *
 *                                                                      *
 * Parameters - none                                                    *
 *                                                                      *
 * Returns - nothing                                                    *
 ************************************************************************/
void SimView3D::renderCells()
{
  // set blending for opaque cells
  glBlendFunc(GL_ONE, GL_ZERO);

  // we want cell's material properties to be determined by color setting
  glEnable(GL_COLOR_MATERIAL);

  // get current list of cells
  vector<Cell *> cell_list = m_tissuep->getCellList();

  // display each cell in list
  for (unsigned int i=0; i<cell_list.size(); i++)
  {
    // get necessary info
    Cell *pc = cell_list[i];
    int type_index = pc->getTypeIndex();
    SimPoint pos = pc->getPosition();
    double radius = m_cellSizes[type_index];
    Color color = cell_palette[type_index];

    // set color, translate, scale & display
    glColor3fv(color.getfv());
    glPushMatrix();
      glTranslatef(pos.getX(), pos.getY(), pos.getZ());
      glScalef(radius, radius, radius);
      gluSphere(quadratic,1,32,32);
    glPopMatrix();
  }

  glDisable(GL_COLOR_MATERIAL);
}

/************************************************************************ 
 * renderMolecules()                                                    *
 *                                                                      *
 * Parameters - none                                                    *
 *                                                                      *
 * Returns - nothing                                                    *
 ************************************************************************/
/*void SimView3D::renderMolecules()
{
  // set blending for translucent molecules
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  int numTypes = m_tissuep->getNumMolTypes();
  double max = 1E-13;		
	  
  for (int n=0; n<numTypes; n++)
  {
    // get concentration data for this molecule type
    const Array3D<Molecule::Conc>& conc = m_tissuep->getConc(n);

    // get base color
    Color color = mol_palette[n];

    // loop through grid cells
    for (int i=1; i<=m_xnum; i++)
      for (int j=1; j<=m_ynum; j++)
        for (int k=1; k<=m_znum; k++)
	{
	  double c = conc.at(i, j, k);

          // scale color according to concentration 
          if (c == 0)                     // nothing to draw
            continue;                     
          else if (c >= max)              // use unadjusted base color
            glColor4fv((color).getfv());
          else                            // scale color by concentration
            glColor4fv((color*(c/max)).getfv());

          // scale, translate by grid cell indices & draw
	  glPushMatrix();
    	    glScalef(m_gridsize, m_gridsize, m_gridsize);
            glTranslatef(i, j, k);
            drawGrid();   
	  glPopMatrix();
	}

  }	// end for each molecule type
}
*/
/************************************************************************ 
 * drawGrid()                                                           *
 *   OpenGL code to draw a grid cell; calling routine must handle 	*
 * translation to appropriate location                                  *
 *                                                                      *
 * Parameters - none                                                    *
 *                                                                      *
 * Returns - nothing                                                    *
 ************************************************************************/
/*void SimView3D::drawGrid()
{
  // draw cube   
  glBegin(GL_QUADS);
  for (unsigned int i=0; i<24; i++)
    glVertex3sv(vertices2[indices2[i]]);
  glEnd();
}
*/

