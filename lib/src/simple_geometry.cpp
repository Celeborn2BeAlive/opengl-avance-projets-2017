#include <glmlv/simple_geometry.hpp>
#include <glm/gtc/constants.hpp>

namespace glmlv
{

SimpleGeometry makeTriangle()
{
    std::vector<Vertex3f3f3f> vertexBuffer =
    {
        { glm::vec3(-0.5, -0.5, 0), glm::vec3(0, 0, 1), glm::vec3(0, 0,0) },
        { glm::vec3(0.5, -0.5, 0), glm::vec3(0, 0, 1), glm::vec3(1, 0,0) },
        { glm::vec3(0., 0.5, 0), glm::vec3(0, 0, 1), glm::vec3(0.5, 1,0) }
    };

    std::vector<uint32_t> indexBuffer =
    {
        0, 1, 2
    };

    return{ vertexBuffer, indexBuffer };
}


SimpleGeometry makeCube(glm::vec3 trans,glm::vec3 size,glm::vec3 C)
{
    std::vector<Vertex3f3f3f> vertexBuffer =
    {
        // Bottom side
        { glm::vec3(-0.5*size.x+trans.x, -0.5*size.y+trans.y, -0.5*size.z+trans.z), glm::vec3(0, -1, 0), C },
        { glm::vec3(0.5*size.x+trans.x, -0.5*size.y+trans.y, -0.5*size.z+trans.z), glm::vec3(0, -1, 0), C },
        { glm::vec3(0.5*size.x+trans.x, -0.5*size.y+trans.y, 0.5*size.z+trans.z), glm::vec3(0, -1, 0), C },
        { glm::vec3(-0.5*size.x+trans.x, -0.5*size.y+trans.y, 0.5*size.z+trans.z), glm::vec3(0, -1, 0), C },
        // Right side
        { glm::vec3(0.5*size.x+trans.x, -0.5*size.y+trans.y, 0.5*size.z+trans.z), glm::vec3(1, 0, 0), C },
        { glm::vec3(0.5*size.x+trans.x, -0.5*size.y+trans.y, -0.5*size.z+trans.z), glm::vec3(1, 0, 0), C },
        { glm::vec3(0.5*size.x+trans.x, 0.5*size.y+trans.y, -0.5*size.z+trans.z), glm::vec3(1, 0, 0), C },
        { glm::vec3(0.5*size.x+trans.x, 0.5*size.y+trans.y, 0.5*size.z+trans.z), glm::vec3(1, 0, 0), C },
        // Back side
        { glm::vec3(0.5*size.x+trans.x, -0.5*size.y+trans.y, -0.5*size.z+trans.z), glm::vec3(0, 0, -1), C },
        { glm::vec3(-0.5*size.x+trans.x, -0.5*size.y+trans.y, -0.5*size.z+trans.z), glm::vec3(0, 0, -1), C },
        { glm::vec3(-0.5*size.x+trans.x, 0.5*size.y+trans.y, -0.5*size.z+trans.z), glm::vec3(0, 0, -1), C },
        { glm::vec3(0.5*size.x+trans.x, 0.5*size.y+trans.y, -0.5*size.z+trans.z), glm::vec3(0, 0, -1), C },
        // Left side
        { glm::vec3(-0.5*size.x+trans.x, -0.5*size.y+trans.y, -0.5*size.z+trans.z), glm::vec3(-1, 0, 0), C },
        { glm::vec3(-0.5*size.x+trans.x, -0.5*size.y+trans.y, 0.5*size.z+trans.z), glm::vec3(-1, 0, 0), C },
        { glm::vec3(-0.5*size.x+trans.x, 0.5*size.y+trans.y, 0.5*size.z+trans.z), glm::vec3(-1, 0, 0), C },
        { glm::vec3(-0.5*size.x+trans.x, 0.5*size.y+trans.y, -0.5*size.z+trans.z), glm::vec3(-1, 0, 0), C },
        // Front side
        { glm::vec3(-0.5*size.x+trans.x, -0.5*size.y+trans.y, 0.5*size.z+trans.z), glm::vec3(0, 0, 1), C },
        { glm::vec3(0.5*size.x+trans.x, -0.5*size.y+trans.y, 0.5*size.z+trans.z), glm::vec3(0, 0, 1), C },
        { glm::vec3(0.5*size.x+trans.x, 0.5*size.y+trans.y, 0.5*size.z+trans.z), glm::vec3(0, 0, 1), C },
        { glm::vec3(-0.5*size.x+trans.x, 0.5*size.y+trans.y, 0.5*size.z+trans.z), glm::vec3(0, 0, 1), C },
        // Top side
        { glm::vec3(-0.5*size.x+trans.x, 0.5*size.y+trans.y, 0.5*size.z+trans.z), glm::vec3(0, 1, 0), C },
        { glm::vec3(0.5*size.x+trans.x, 0.5*size.y+trans.y, 0.5*size.z+trans.z), glm::vec3(0, 1, 0), C },
        { glm::vec3(0.5*size.x+trans.x, 0.5*size.y+trans.y, -0.5*size.z+trans.z), glm::vec3(0, 1, 0), C },
        { glm::vec3(-0.5*size.x+trans.x, 0.5*size.y+trans.y, -0.5*size.z+trans.z), glm::vec3(0, 1, 0), C }
    };

    std::vector<uint32_t> indexBuffer =
    {
        0, 1, 2,
        0, 2, 3,
        4, 5, 6,
        4, 6, 7,
        8, 9, 10,
        8, 10, 11,
        12, 13, 14,
        12, 14, 15,
        16, 17, 18,
        16, 18, 19,
        20, 21, 22,
        20, 22, 23
    };

    return{ vertexBuffer, indexBuffer };
}

SimpleGeometry makeSphere(uint32_t subdivLongitude, glm::vec3 C)
{
    const auto discLong = subdivLongitude;
    const auto discLat = 2 * discLong;

    float rcpLat = 1.f / discLat, rcpLong = 1.f / discLong;
    float dPhi = glm::pi<float>() * 2.f * rcpLat, dTheta = glm::pi<float>() * rcpLong;

    std::vector<Vertex3f3f3f> vertexBuffer;

    for (uint32_t j = 0; j <= discLong; ++j)
    {
        float cosTheta = cos(-glm::half_pi<float>() + j * dTheta);
        float sinTheta = sin(-glm::half_pi<float>() + j * dTheta);

        for (uint32_t i = 0; i <= discLat; ++i) {
            glm::vec3 coords;

            coords.x = sin(i * dPhi) * cosTheta;
            coords.y = sinTheta;
            coords.z = cos(i * dPhi) * cosTheta;

            vertexBuffer.emplace_back(coords, coords, C);
        }
    }

    std::vector<uint32_t> indexBuffer;

    for (uint32_t j = 0; j < discLong; ++j)
    {
        uint32_t offset = j * (discLat + 1);
        for (uint32_t i = 0; i < discLat; ++i)
        {
            indexBuffer.push_back(offset + i);
            indexBuffer.push_back(offset + (i + 1));
            indexBuffer.push_back(offset + discLat + 1 + (i + 1));

            indexBuffer.push_back(offset + i);
            indexBuffer.push_back(offset + discLat + 1 + (i + 1));
            indexBuffer.push_back(offset + i + discLat + 1);
        }
    }

    return{ vertexBuffer, indexBuffer };
}

SimpleGeometry makeScene(){
    SimpleGeometry scene = makeSphere(32,glm::vec3(1,1,1));
    uint32_t offset = scene.vertexBuffer.size();
    SimpleGeometry c = makeCube(glm::vec3(-4,0,0),glm::vec3(1,1,1),glm::vec3(1,1,1));
    int i;
    for(i=0;i<c.indexBuffer.size();i++){
        c.indexBuffer[i]+=offset;
        scene.indexBuffer.push_back(c.indexBuffer[i]);
    }
    for(i=0;i<c.vertexBuffer.size();i++){
        scene.vertexBuffer.push_back(c.vertexBuffer[i]);
    }

    offset = scene.vertexBuffer.size();
    SimpleGeometry b = makeCube(glm::vec3(-2,-2,0),glm::vec3(10,0.1,10),glm::vec3(1,1,1));
    for(i=0;i<b.indexBuffer.size();i++){
        b.indexBuffer[i]+=offset;
        scene.indexBuffer.push_back(b.indexBuffer[i]);
    }
    for(i=0;i<b.vertexBuffer.size();i++){
        scene.vertexBuffer.push_back(b.vertexBuffer[i]);
    }

    offset = scene.vertexBuffer.size();
    SimpleGeometry u = makeCube(glm::vec3(-2,2,0),glm::vec3(10,0.1,10),glm::vec3(1,1,1));
    for(i=0;i<u.indexBuffer.size();i++){
        u.indexBuffer[i]+=offset;
        scene.indexBuffer.push_back(u.indexBuffer[i]);
    }
    for(i=0;i<u.vertexBuffer.size();i++){
        scene.vertexBuffer.push_back(u.vertexBuffer[i]);
    }

    offset = scene.vertexBuffer.size();
    SimpleGeometry r = makeCube(glm::vec3(2,0,0),glm::vec3(0.1,10,10),glm::vec3(1,0,0));
    for(i=0;i<r.indexBuffer.size();i++){
        r.indexBuffer[i]+=offset;
        scene.indexBuffer.push_back(r.indexBuffer[i]);
    }
    for(i=0;i<r.vertexBuffer.size();i++){
        scene.vertexBuffer.push_back(r.vertexBuffer[i]);
    }

    offset = scene.vertexBuffer.size();
    SimpleGeometry l = makeCube(glm::vec3(-5,0,0),glm::vec3(0.1,10,10),glm::vec3(0,1,0));
    for(i=0;i<l.indexBuffer.size();i++){
        l.indexBuffer[i]+=offset;
        scene.indexBuffer.push_back(l.indexBuffer[i]);
    }
    for(i=0;i<l.vertexBuffer.size();i++){
        scene.vertexBuffer.push_back(l.vertexBuffer[i]);
    }

    offset = scene.vertexBuffer.size();
    SimpleGeometry back = makeCube(glm::vec3(-2,0,-2),glm::vec3(10,10,0.1),glm::vec3(1,1,1));
    for(i=0;i<back.indexBuffer.size();i++){
        back.indexBuffer[i]+=offset;
        scene.indexBuffer.push_back(back.indexBuffer[i]);
    }
    for(i=0;i<back.vertexBuffer.size();i++){
        scene.vertexBuffer.push_back(back.vertexBuffer[i]);
    }


    return scene;
}

}
