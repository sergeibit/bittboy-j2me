/*
 * MIDPath - Copyright (C) 2006-2007 Guillaume Legris, Mathieu Legris
 * 
 * Odonata - Copyright (C) 2002-2006 Stephane Meslin-Weber <steph@tangency.co.uk>
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License version
 * 2 only, as published by the Free Software Foundation. 
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License version 2 for more details (a copy is
 * included at /legal/license.txt). 
 * 
 * You should have received a copy of the GNU General Public License
 * version 2 along with this work; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA 
 * 
 * Linking this library statically or dynamically with other modules is
 * making a combined work based on this library.  Thus, the terms and
 * conditions of the GNU General Public License cover the whole
 * combination.
 *
 * As a special exception, the copyright holders of this library give you
 * permission to link this library with independent modules to produce an
 * executable, regardless of the license terms of these independent
 * modules, and to copy and distribute the resulting executable under
 * terms of your choice, provided that you also meet, for each linked
 * independent module, the terms and conditions of the license of that
 * module.  An independent module is a module which is not derived from
 * or based on this library.  If you modify this library, you may extend
 * this exception to your version of the library, but you are not
 * obligated to do so.  If you do not wish to do so, delete this
 * exception statement from your version.
 */
package org.thenesis.midpath.font.bdf;

import java.io.Reader;
import java.util.Hashtable;

public class BDFFontContainer {
    //private static final PropertyResourceBundle charMapper = (PropertyResourceBundle)PropertyResourceBundle.getBundle("uk.co.tangency.odonata.font.bdf.mappings");
    public static final Hashtable charMapper;
    
    static {
    	charMapper = new Hashtable();
    	charMapper.put(".undef", String.valueOf('\u0219'));
    	charMapper.put(".null", String.valueOf('\u0000'));
    	charMapper.put("space", String.valueOf('\u0020'));
    	charMapper.put("period", String.valueOf('.'));
    	charMapper.put("one", String.valueOf('1'));
    	charMapper.put("two", String.valueOf('2'));
    	charMapper.put("three", String.valueOf('3'));
    	charMapper.put("four", String.valueOf('4'));
    	charMapper.put("five", String.valueOf('5'));
    	charMapper.put("six", String.valueOf('6'));
    	charMapper.put("seven", String.valueOf('7'));
    	charMapper.put("eight", String.valueOf('8'));
    	charMapper.put("nine", String.valueOf('9'));
    	charMapper.put("zero", String.valueOf('0'));
    }
    
    static final Object LOCK = new Object();
    private static final long serialVersionUID = -2156798287434571634L;

    // Actual names of defined PostScript name fields:
    //   fndry fmly wght slant sWdth adstyl pxlsz ptSz resx resy
    //   spc avgWdth rgstry encdng
    // decoded into english below

    public static final int FOUNDRY = 0;
    public static final int FAMILY = 1;
    public static final int WEIGHT = 2;
    public static final int SLANT = 3;
    public static final int SWIDTH = 4;
    public static final int ADSTYL = 5;
    public static final int PIXELSIZE = 6;
    public static final int POINTSIZE = 7;
    public static final int HORIZONTAL = 8;
    public static final int VERTICAL = 9;
    public static final int SPACING = 10;
    public static final int AVERAGEWIDTH = 11;
    public static final int REGISTRY = 12;
    public static final int ENCODING = 13;
    
    
    
//  static int bitSwap(int n) {
//      n = ((n >> 1) & 0x55) | ((n << 1) & 0xaa);
//      n = ((n >> 2) & 0x33) | ((n << 2) & 0xcc);
//      n = ((n >> 4) & 0x0f) | ((n << 4) & 0xf0);
//      return n;
//  }
    
    private int style;
            
    public static final BDFFontContainer createFont(Reader r) throws Exception {
        BDFParser parser = new BDFParser(r);
        BDFFontContainer font = parser.createFont();
        return font;
    }
    
    public static int fill(int num) {
        return (1 << num+1)-1;
    }
    
    public static void init() {
        synchronized(LOCK) {
        }
    }

    public boolean equals(Object obj) {
        if(obj instanceof BDFFontContainer) {
            
        }
        return false;
    }
    
    public static void main(String[] args) {
        
    }

    private BDFParser.Rectangle boundingBox = new BDFParser.Rectangle();

    private String[] comments;

    private int contentVersion;

    private int depth = 1;

    private String[] fontName;

    //private HashMap glyphMapper = new HashMap();
    private Hashtable glyphMapper = new Hashtable();
    
    private BDFGlyph[] glyphs;
    
    // FIXME: actually start using this.
    private int metricsSet = 0;

    private String[] properties;
    
    private BDFParser.Dimension resolution = new BDFParser.Dimension();

    private BDFParser.Version version = new BDFParser.Version();

    private String postScriptName;
    
    private int size;
    
    private String family;
    
    BDFFontContainer(String name[], int style, int size) {
        postScriptName = getPostScriptName(name);
        this.style = style;
        this.size = size;
        this.family = name[FAMILY];
        fontName = name;
    }
    
    public String getName() {
        return postScriptName;
    }
    
    public String getFamily() {
        return family;
    }

    public BDFParser.Rectangle getBoundingBox() {
        return boundingBox;
    }
    
    public int getStyle() {
        return style;
    }

    public String[] getComments() {
        return comments;
    }

    public int getContentVersion() {
        return contentVersion;
    }

    public int getDepth() {
        return depth;
    }
    
    public int getSize() {
        return size;
    }

    public BDFMetrics getFontMetrics() {
        return new BDFMetrics(this);
    }

    public BDFGlyph getGlyph(char ch) {
        BDFGlyph g = (BDFGlyph)glyphMapper.get(""+(ch));
        if(g==null) {
            g = (BDFGlyph)glyphMapper.get(""+'\u0020');
        }
        return g;
    }

    public BDFGlyph[] getGlyphs() {
        return glyphs;
    }

    public String[] getProperties() {
        return properties;
    }

	public BDFParser.Dimension getResolution() {
        return resolution;
    }

    public BDFParser.Version getVersion() {
        return version;
    }
    
  String lookup(String s) {
	  String result = (String)charMapper.get(".undef");
	  if(s.length()==1) {
		  result = ""+(s.charAt(0));
	  } else if(charMapper.get(s) != null) {
		  result = (String)charMapper.get(s);
  }

  return result;
}

//    String lookup(String s) {
//        String result = charMapper.getString(".undef");
//        if(s.length()==1) {
//            result = ""+(s.charAt(0));
//        } else if(charMapper.handleGetObject(s)!=null) {
//            result =charMapper.getString(s);
//        }
//
//        return result;
//    }

    public void setBoundingBox(int x, int y, int width, int height) {
        boundingBox.setBounds(x, y, width, height);
    }

    public void setBoundingBox(BDFParser.Rectangle boundingBox) {
        this.boundingBox = boundingBox;
    }

    public void setCharCount(int count) {
        glyphs = new BDFGlyph[count];
    }

    public void setComments(String[] comments) {
        this.comments = comments;
    }

    public void setContentVersion(int version) {
        this.contentVersion = version;
    }

    public void setDepth(int depth) {
        this.depth = depth;
    }

    public void setGlyphs(BDFGlyph[] glyphs) {
        this.glyphs = glyphs;
        for(int i=0;i<glyphs.length;i++) {
            glyphs[i].init(this);
            glyphMapper.put(lookup(glyphs[i].getName()),glyphs[i]);
        }
    }

    public void setMetricsSet(int set) {
        System.err.println("Metrics set: " + set);
    }

    public void setProperties(String[] properties) {
        this.properties = properties;
    }

	public void setResolution(BDFParser.Dimension resolution) {
		this.resolution = resolution;
	}
    
    public void setResolution(int xres, int yres) {
		resolution.setSize(xres, yres);
	}

    public void setVersion(BDFParser.Version version) {
        this.version = version;
    }

    public void setVersion(int major, int minor) {
        version.setVersion(major, minor);
    }
    
    public static final String getPostScriptName(String[] str) {
        StringBuffer string = new StringBuffer();
        for(int i=0;i<str.length;i++) {
            string.append("-");
            string.append(str[i]);
        }
        return string.toString();
    }
    
    public String toString() {
        String styleStr = ((style & BDFParser.BOLD) != 0 ? "Bold " : "")
                + ((style & BDFParser.ITALIC) != 0 ? "Italic" : "");
        
        if ("".equals(styleStr)) {
            styleStr = "Plain";
        }
        
        return getClass().getName() + "[name=" + fontName[FAMILY] + ", style="
                + styleStr + ", size=" + size + "pt, depth=" + depth + "bpp]";
    }
}