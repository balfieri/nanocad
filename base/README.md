Class Hierarchy (look at *.h and *.cpp files of similar name):

<pre>
Misc             -- miscellaneous functions
Color            -- RGB colors by name or number
Config           -- configuration variables
World            -- 3D world
    Vertex          -- single 3D vertex   (defined in World.h)
    Triangle        -- single 3D triangle (defined in World.h)
    Entity              -- 2D/3D object in the world (base class for all world objects)
        Box                 -- box (3D)
        Rectangle           -- rectangle (2D)
        [others to be added]
Sys              -- system dependencies (files, OpenGL, etc.)
    Geom            -- batch of 3D geometry (defined in Sys.h, points to Vertex and Triangle data)
    Text2D          -- 2D text (defined in Sys.h)
    Sys_glut.cpp    -- OpenGL/glut implementation of Sys.h

List            -- simiilar to std::vector, but very high-performance and intended for JS or Python type dynamic structures
Hash            -- similar  to std::map,    but ditto
    NodeIO.cpp  -- reads in .json files into Lists and Hashs (with recursive structures allowed)
