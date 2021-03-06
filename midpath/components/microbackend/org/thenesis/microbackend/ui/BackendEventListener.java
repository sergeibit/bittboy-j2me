/*
 * MIDPath - Copyright (C) 2006-2008 Guillaume Legris, Mathieu Legris
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License version
 * 2 only, as published by the Free Software Foundation. 
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License version 2 for more details. 
 * 
 * You should have received a copy of the GNU General Public License
 * version 2 along with this work; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA  
 */
package org.thenesis.microbackend.ui;

public interface BackendEventListener {
	
	public void keyPressed(int keycode, char c, int modifiers);
	public void keyReleased(int keycode, char c, int modifiers);
	
	public void mouseMoved(int x, int y, int modifiers);
	public void mousePressed(int x, int y, int modifiers);
	public void mouseReleased(int x, int y, int modifiers);
	
	public void windowClosed();


}
