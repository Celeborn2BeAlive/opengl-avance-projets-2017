/**
 * @file Provides the implementation for a renderer.
 * A renderer responsibility is to manage and hold buffers and shaders for rendering in OpenGL
 *
 * @author Olivier Falconnet
 */

#pragma once

#include <ANGL/Common.hpp>
#include <ANGL/Core.hpp>
#include <glmlv/GLProgram.hpp>
#include <ANGL/Graphics/ModelGeometry.hpp>
#include <glmlv/ViewController.hpp>

namespace ANGL {

    struct PointLight {
        glm::vec3 position;
        glm::vec3 color;
        float intensity;
        bool enabled = true;

        //float boundingSphereRadius();
    };

    static glm::vec3 computeDirectionVector(float phiRadians, float thetaRadians)
    {
        const auto cosPhi = glm::cos(phiRadians);
        const auto sinPhi = glm::sin(phiRadians);
        const auto sinTheta = glm::sin(thetaRadians);
        return glm::vec3(sinPhi * sinTheta, glm::cos(thetaRadians), cosPhi * sinTheta);
    }


    class Renderer {

        friend class ModelGeometry;

    public :

        Renderer();

        //~Renderer();

        /**
         *  @return {Renderer*} a pointer to the most recently instantiated renderer
         */
        static Renderer* getRenderer();


        /**
         *
         */
        void Init();

        /**
         *
         */
        void Draw(ModelGeometry pModel);

        /**
         *
         */
        //void LoadTexture(std::string &pTextureName, std::string &pBasepath);

        static const GLint gVBOBindingIndex = 0; // Arbitrary choice between 0 and glGetIntegerv(GL_MAX_VERTEX_ATTRIB_BINDINGS)

        // VAO attributes binding locations
        static const GLuint gPositionAttr_binding_geometry = 0;
        static const GLuint gNormalAttr_binding = 1;
        static const GLuint gTangentAttr_binding = 2;
        static const GLuint gTexCoordsAttr_binding = 3;

        static const GLuint gPositionAttr_binding_shading = 0;

        static const GLuint gPointLightPosition_binding = 4;
        static const GLuint gPointLightColor_binding = 5;
        static const GLuint gPointLightIntensity_binding = 6;
        static const GLuint gPointLightEnabled_binding = 7;


        float m_near = 0.1f;
        float m_far = 1000.f;
        float m_ViewControllerSpeed = 80.f;
        float m_ViewControllerRotationSpeed = 0.01f;

        glmlv::ViewController mViewController;


        float m_DirLightPhiAngleDegrees = 0.f;
        float m_DirLightThetaAngleDegrees = 45.f;
        glm::vec3 m_DirLightDirection = computeDirectionVector(m_DirLightPhiAngleDegrees, m_DirLightThetaAngleDegrees);
        glm::vec3 directionalLightColor = {1, 1, 1};
        float directionalLightIntensity = 1.f;

        bool enablePointLights = true;
        std::vector<PointLight> pointLights = {
                {{64, 8, 0}, {1, 0, 0}, 100.f, true},
                {{0, 16, 0}, {0, 1, 0}, 750.f, true},
                {{0, 8, 32}, {0, 0, 1}, 500.f, true}
        };

    private :

        // Auxiliary methods for Init()
        void CompileGeometry();
        void CompileSSAO();
        void CompileShading();
        void CompileDepth();
        void CompilePosition();

        void CreateSSBOs();

        void CreateGBuffers();
        void CreateSSAOTextures();
        void CreateFrameBuffer_Geometry();
        void CreateFrameBuffer_SSAO();

        void CreateSSAOKernel();
        void CreateSSAONoise();

        void CreateDisplayMesh(bool half = false);


        // Auxiliary methods for Draw()
        void UpdatePointLightSSBO(const std::vector<PointLight> & pointLights, const glm::mat4 & viewMatrix);

        void DrawGUI_Renderer();
        void DrawGUI_Lights();

//        void Pass_Geometry();
//        void Pass_AmbientOcclusion();
//        void Pass_Shading();
//
//        void Display_DepthBuffer();
//        void Display_PostionBuffers();
//        void Display_GBuffer(GBufferTextureType gBuffer);
//        void Display_SSAOBuffer();


        void DrawGUI();


        static Renderer* gInstance;

        glmlv::fs::path mShadersRootPath;

        GLuint mTextureSampler;

        float mClearColor[3] = {0.0, 1.0, 1.0};

        //float mSSAORadius = 0.5f;
        float mSSAORadius = 10.f;
        float mSSAOBias = 0.025f;

        // For Deferred Shading
        enum GBufferTextureType
        {
            GPosition = 0,
            GNormal,
            GAmbient,
            GDiffuse,
            GGlossyShininess,
            GDepth, // On doit créer une texture de depth mais on écrit pas directement dedans dans le FS. OpenGL le fait pour nous (et l'utilise).
            GBufferTextureCount,
            SSAO,   // Not actually GBuffer texture
            Shaded,
        };
        GBufferTextureType m_CurrentlyDisplayed = Shaded;

        GLuint m_GBufferTextures[GBufferTextureCount];
        const GLenum m_GBufferTextureFormat[GBufferTextureCount] = {
                GL_RGB32F, GL_RGB32F, GL_RGB32F, GL_RGB32F, GL_RGBA32F, GL_DEPTH_COMPONENT32F };
        const char * m_GBufferTexNames[GBufferTextureCount] =
                { "position", "normal", "ambient", "diffuse", "glossyShininess", "depth" };
        GLuint m_GBufferFBO;

        GLuint m_SSAOTexture;
        const GLenum m_SSAOTextureFormat = GL_R32F;
        GLuint m_SSAOFBO;

        std::vector<glm::vec3> m_SSAOKernel;
        GLuint m_SSAONoiseTexture;
        glm::ivec2 m_SSAONoiseTextureSize;

        GLuint m_displayVB0;
        GLuint m_displayVAO;


        //___SHADER PROGRAMS AND UNIFORM LOCATIONS

        //____Geometry Pass____

        glmlv::GLProgram m_geometryProgram;

        // Geometry program uniform locations
        GLint m_uModelViewProjMatrix_location;
        GLint m_uModelViewMatrix_location;
        GLint m_uNormalMatrix_location;

        GLint m_uKaSampler_location;
        GLint m_uKdSampler_location;
        GLint m_uKsSampler_location;
        GLint m_uNsSampler_location;
        GLint m_udSampler_location;
        GLint m_uNormalSampler_location;

        GLint m_uKaMap_location;
        GLint m_uKdMap_location;
        GLint m_uKsMap_location;
        GLint m_uNsMap_location;
        GLint m_udMap_location;
        GLint m_uNormalMap_location;

        GLint m_uKa_location;
        GLint m_uKd_location;
        GLint m_uKs_location;
        GLint m_uNs_location;
        GLint m_ud_location;
        // Normal is sent through attributes


        //____Ambient Occlusion Pass____

        glmlv::GLProgram m_SSAOProgram;

        GLint m_uGPositionSampler_location_SSAO;
        GLint m_uGNormalSampler_location_SSAO;
        GLint m_uNoiseSampler_location_SSAO;

        GLint m_uKernel_location_SSAO;
        GLint m_uProjMatrix_location_SSAO;

        GLint m_uFramebufferSize_location_SSAO;
        GLint m_uNoiseTileSize_location_SSAO;

        GLint m_uRadius_location_SSAO;
        GLint m_uBias_location_SSAO;


        //____Shading Pass____

        glmlv::GLProgram m_shadingProgram;

        // Shading program uniforms locations

        GLint m_uDirectionalLightDir_location;
        GLint m_uDirectionalLightColor_location;
        GLint m_uDirectionalLightIntensity_location;

        GLint m_uEnablePointLights_location;
        GLint m_uNbPointLights_location;

        GLuint m_pointLightPositionSSBO;
        GLuint m_pointLightColorSSBO;
        GLuint m_pointLightIntensitySSBO;
        GLuint m_pointLightEnabledSSBO;

        GLint m_uGPositionSampler_location_shading;
        GLint m_uGNormalSampler_location_shading;
        GLint m_uGAmbientSampler_location;
        GLint m_uGDiffuseSampler_location;
        GLint m_uGlossyShininessSampler_location;
        GLint m_uGDepthSampler_location_shading;
        GLint m_uSSAOSampler_location_shading;


        //____Display Depth Buffer____

        glmlv::GLProgram m_depthProgram;

        // Depth program uniform locations
        GLint m_uAdjustment_location_depth;
        GLint m_uGDepthSampler_location_depth;

        GLuint m_depthTexture;


        //____Display Position Buffer____

        glmlv::GLProgram m_positionProgram;

        // Position program uniforms locations
        GLint m_uAdjustment_location_position;
        GLint m_uGPositionSampler_location_position;
    };

} // namespace ANGL
