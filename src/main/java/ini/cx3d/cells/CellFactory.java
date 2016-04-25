/*
Copyright (C) 2009 Frédéric Zubler, Rodney J. Douglas,
Dennis Göhlsdorf, Toby Weston, Andreas Hauri, Roman Bauer,
Sabina Pfister & Adrian M. Whatley.

This file is part of CX3D.

CX3D is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

CX3D is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with CX3D.  If not, see <http://www.gnu.org/licenses/>.
*/

package ini.cx3d.cells;

import ini.cx3d.localBiology.interfaces.SomaElement;
import ini.cx3d.localBiology.factory.SomaElementFactory;
import ini.cx3d.physics.interfaces.PhysicalSphere;
import ini.cx3d.physics.factory.PhysicalSphereFactory;
import ini.cx3d.simulations.ECMFacade;
import ini.cx3d.simulations.interfaces.ECM;
import ini.cx3d.spatialOrganization.SpatialOrganizationNode;
import ini.cx3d.spatialOrganization.interfaces.SpaceNode;
import ini.cx3d.swig.biology.biology;
import ini.cx3d.utilities.Matrix;

import java.util.Vector;

/**
 * <code>CellFacory</code> generates a new  <code>Cell</code>, <code>SomaElement</code>, 
 * <code>PhysicalSphere</code> and <code>SpatialOrganizationNode</code>.  We set than the
 * massLocation and cell color.
 * We can generate a single <code>Cell</code> or a list of <code>Cell</code> distributed
 * uniformly.
 * @author rjd/sabina
 *
 */
public class CellFactory {
    
    /* Reference to the ECM. */
    private static ini.cx3d.simulations.interfaces.ECM ecm = ECMFacade.getInstance();

	/**
	 * <code>CellFactory</code> constructor.
	 */
    public CellFactory() { 
    }

    static {
        ini.cx3d.swig.biology.Cell.setECM(ECMFacade.getInstance());
    }
    
    /**
     * Generates a single cell at the specified position.
     * @param cellOrigin
     * @return
     */
    public static ini.cx3d.cells.interfaces.Cell getCellInstance(double[] cellOrigin) {
    	if(biology.useNativeCellFactory) {
            return ini.cx3d.swig.biology.CellFactory.getCellInstance(cellOrigin, ECMFacade.getInstance());
        }
    	// Create new cell
        ini.cx3d.cells.interfaces.Cell cell = new Cell();
        SomaElement soma = SomaElementFactory.create();
        cell.setSomaElement(soma);        
        PhysicalSphere ps = PhysicalSphereFactory.create();
        soma.setPhysical(ps);
        SpatialOrganizationNode<ini.cx3d.physics.interfaces.PhysicalNode> son = ecm.getSpatialOrganizationNodeInstance(cellOrigin.clone(), ps);
        ps.setSoNode((SpaceNode) son);
        
        // Add cell to ECM instance
        ecm.addPhysicalSphere(soma.getPhysicalSphere());
        
        // Set cell properties
        ps.setMassLocation(cellOrigin);
        ps.setColor(ecm.cellTypeColor(cell.getType()));
        return cell;
    }
    
   /**
    * Generates a 2D grid of cells according according to the desired number of cells along 
    * the y and x axes. Cell position can be randomized by increasing the standard deviation of
    * the Gaussian noise distribution. 
    * @param xmin 
    * @param xmax
    * @param ymin
    * @param ymax
    * @param nx: Number of cells along the x axis
    * @param ny: Number of cells along the y axis
    * @param noiseStd: Gaussian noise standard deviation
    * @return cellList
    */
   public static Vector<ini.cx3d.cells.interfaces.Cell> get2DCellGrid(double xmin, double xmax, double ymin,
		   double ymax, double zpos, int nx, int ny, double noiseStd) {
	   
	   // Insert all generated cells in a vector
       Vector<ini.cx3d.cells.interfaces.Cell> cellList = new Vector<ini.cx3d.cells.interfaces.Cell>();
       double dx = (xmax-xmin)/(1+nx);
       double dy = (ymax-ymin)/(1+ny);
       
       // Generate cells
       for (int i=1; i < nx+1; i++) {
       	for (int j=1; j < ny+1; j++) {
       		double[] newLocation = {
       				xmin+i*dx+ecm.getGaussianDouble(0, noiseStd), 
       				ymin+j*dy+ecm.getGaussianDouble(0, noiseStd), 
       				zpos + ecm.getGaussianDouble(0, noiseStd)};
       		Matrix.print(newLocation);
       		ini.cx3d.cells.interfaces.Cell cell = getCellInstance(newLocation);
       		cellList.add(cell);
       	}
       }
       return cellList;
   }
   
   /**
    * Generates a 3D grid of cells according to the desired number of cells along 
    * the y, x and z axes. Cell position can be randomized by increasing the standard deviation of
    * the Gaussian noise distribution. 
    * @param xmin 
    * @param xmax
    * @param ymin
    * @param ymax
    * @param zmin
    * @param zmax
    * @param nx: Number of cells along the x axis
    * @param ny: Number of cells along the y axis
    * @param nz: Number of cells along the z axis
    * @return cellList
    */
   public static Vector<ini.cx3d.cells.interfaces.Cell> get3DCellGrid(double xmin, double xmax, double ymin,
		   double ymax, double zmin, double zmax, int nx, int ny, int nz, double noiseXYStd, double noiseZStd) {
	   
	   // Insert all generated cells in a vector
       Vector<ini.cx3d.cells.interfaces.Cell> cellList = new Vector<ini.cx3d.cells.interfaces.Cell>();
       double dx = (xmax-xmin)/(1+nx);
       double dy = (ymax-ymin)/(1+ny);
       double dz = (zmax-zmin)/(1+nz);
       
       // Generate cells
       for (int i=1; i < nx+1; i++) {
       	for (int j=1; j < ny+1; j++) {
       		for (int k=1; k < nz+1; k++) {
       			double[] newLocation = {
       					xmin+i*dx+ecm.getGaussianDouble(0, noiseXYStd), 
       					ymin+j*dy+ecm.getGaussianDouble(0, noiseXYStd), 
       					zmin+k*dz+ecm.getGaussianDouble(0, noiseZStd)};
       			ini.cx3d.cells.interfaces.Cell cell = getCellInstance(newLocation);
       			cellList.add(cell);
       		}
       	}
       }
       return cellList;
   }


}