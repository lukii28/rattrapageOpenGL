#ifndef ASSIMP_H
#define ASSIMP_H

#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../shader_helper.h"
#include "../drawable.h"

#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#define aisgl_min(x, y) (x < y ? x : y)
#define aisgl_max(x, y) (y > x ? y : x)

class AssimpLoaded : public Drawable
{
public:
    void init(GLuint pid, const char *path)
    {

        _pid = pid;
        if (_pid == 0)
            exit(-1);

        glUseProgram(_pid);
        scene = NULL;
        scene_list = 0;

        scene = aiImportFile(path, aiProcessPreset_TargetRealtime_MaxQuality);
        const C_STRUCT aiMesh *mesh = scene->mMeshes[0];

        //std::cout << mesh->mNumVertices << std::endl;
        _num_faces=mesh->mNumFaces;

        GLfloat indexed_position[mesh->mNumVertices * 3];
        GLfloat per_surface_normals[mesh->mNumVertices * 3];

        GLuint index[mesh->mNumFaces * 3];

        for (int i = 0; i < mesh->mNumVertices; i++)
        {
            indexed_position[i * 3 + 0] = mesh->mVertices[i][0];
            indexed_position[i * 3 + 1] = mesh->mVertices[i][1];
            indexed_position[i * 3 + 2] = mesh->mVertices[i][2];
            //std::cout << "vertex " << i << ": " << mesh->mVertices[i][0] << " " << mesh->mVertices[i][1] << " " << mesh->mVertices[i][2] << std::endl;
        }

        for (int i = 0; i < mesh->mNumFaces; i++)
        {
            auto face = mesh->mFaces[i];
            index[i * 3] = face.mIndices[0];
            index[i * 3 + 1] = face.mIndices[1];
            index[i * 3 + 2] = face.mIndices[2];
            //std::cout << "face " << i << ": " << face.mIndices[0] << " " << face.mIndices[1] << " " << face.mIndices[2] << std::endl;
        }

        /*index[6*3]=6;
            index[6*3+1]=0;
            index[6*3+2]=20;*/
        
        for (int i = 0; i < mesh->mNumVertices; i++)
        {
            per_surface_normals[i * 3] = mesh->mNormals[i][0];
            per_surface_normals[i * 3 + 1] = mesh->mNormals[i][1];
            per_surface_normals[i * 3 + 2] = mesh->mNormals[i][2];
            //std::cout << "normals " << i << ": " << mesh->mNormals[i][0] << " " << mesh->mNormals[i][1] << " " << mesh->mNormals[i][2] << std::endl;
        }
        float vertices[] = {
            0.5f, 0.5f, 0.0f,   // top right
            0.5f, -0.5f, 0.0f,  // bottom right
            -0.5f, -0.5f, 0.0f, // bottom left
            -0.5f, 0.5f, 0.0f   // top left
        };
        unsigned int indices[] = {
            // note that we start from 0!
            0, 1, 3, // first Triangle
            1, 2, 3  // second Triangle
        };

        glGenVertexArrays(1, &_vao);
        glGenBuffers(1, &_vbo);
        glGenBuffers(1, &_vbo_sur_norm);
        glGenBuffers(1, &_vbo_vert_norm);
        glGenBuffers(1, &_ebo);
        // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
        glBindVertexArray(_vao);

        glBindBuffer(GL_ARRAY_BUFFER, _vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(indexed_position), indexed_position, GL_STATIC_DRAW);

        GLuint id_pos = glGetAttribLocation(_pid, "position");
        glEnableVertexAttribArray(id_pos);
        glVertexAttribPointer(id_pos, 3, GL_FLOAT, GL_FALSE, 0, NULL);

        glBindBuffer(GL_ARRAY_BUFFER, _vbo_sur_norm);
        glBufferData(GL_ARRAY_BUFFER, sizeof(per_surface_normals), per_surface_normals, GL_STATIC_DRAW);

        id_pos = glGetAttribLocation(_pid, "surface_normal");
        glEnableVertexAttribArray(id_pos);
        glVertexAttribPointer(id_pos, 3, GL_FLOAT, GL_FALSE, 0, NULL);

        glBindBuffer(GL_ARRAY_BUFFER, _vbo_vert_norm);
        glBufferData(GL_ARRAY_BUFFER, sizeof(per_surface_normals), per_surface_normals, GL_STATIC_DRAW);

        id_pos = glGetAttribLocation(_pid, "vertex_normal");
        glEnableVertexAttribArray(id_pos);
        glVertexAttribPointer(id_pos, 3, GL_FLOAT, GL_FALSE, 0, NULL);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index), index, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);

        // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // remember: do NOT unbind the EBO while a VAO is active as the bound element buffer object IS stored in the VAO; keep the EBO bound.
        // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
        // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
        glBindVertexArray(0);
    }

    void draw(glm::mat4x4 model, glm::mat4x4 view, glm::mat4x4 projection)
    {
        glUseProgram(_pid);
        glBindVertexArray(_vao);

        glUniformMatrix4fv(glGetUniformLocation(_pid, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(_pid, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(_pid, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        //std::cout<<color[0]<<" "<<color[1]<<" "<<color[2]<<std::endl;

        glUniform3f(glGetUniformLocation(_pid, "color"),color[0],color[1],color[2]);

        glDrawElements(GL_TRIANGLES, 3 * _num_faces, GL_UNSIGNED_INT, 0);

        glUseProgram(0);
        glBindVertexArray(0);
    }

    void set_color(float r, float g, float b)
    {
        color[0] = r;
        color[1] = g;
        color[2] = b;
    }

    void draw()
    {
        glUseProgram(_pid);
        glBindVertexArray(_vao);

        glUniform3fv(glGetUniformLocation(_pid, "light_position"), 1, this->light_position);
        glUniform3fv(glGetUniformLocation(_pid, "camera_position"), 1, this->camera_position);

        glUniform3fv(glGetUniformLocation(_pid, "shape_color"), 1, this->color);
        glUniform4fv(glGetUniformLocation(_pid, "clip_coord"), 1, this->clip_coord);
        glUniform1ui(glGetUniformLocation(_pid, "shadow_mapping_effect"), this->shadow_mapping_effect);
        glUniform1ui(glGetUniformLocation(_pid, "shadow_buffer_tex_size"), this->shadow_buffer_texture_width); // width == height in this case
        glUniform1ui(glGetUniformLocation(_pid, "window_width"), this->window_width);
        glUniform1ui(glGetUniformLocation(_pid, "window_height"), this->window_height);

        glUniform3fv(glGetUniformLocation(_pid, "sun_dir"), 1, this->sun_dir);
        glUniform3fv(glGetUniformLocation(_pid, "sun_col"), 1, this->sun_col);

        if (has_shadow_buffer)
        {
            glUniformMatrix4fv(glGetUniformLocation(_pid, "shadow_matrix"), 1, GL_FALSE, glm::value_ptr(this->shadow_matrix));

            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, _shadow_texture_id);
            GLuint tex_id = glGetUniformLocation(_pid, "shadow_buffer_tex");
            glUniform1i(tex_id, 1 /*GL_TEXTURE0*/);
        }

        draw(this->model_matrix, this->view_matrix, this->projection_matrix);

        if (has_shadow_buffer)
        {
            glBindTexture(GL_TEXTURE_2D, 0);
        }

        glUseProgram(0);
        glBindVertexArray(0);
    }

    void cleanup()
    {
        glDeleteBuffers(1, &_vbo);
        glDeleteBuffers(1, &_ebo);
        glDeleteVertexArrays(1, &_vao);
        glDeleteProgram(_pid);
    }

protected:
    GLuint _vao;
    GLuint _vbo;
    GLuint _ebo;
    GLuint _vbo_sur_norm;
    GLuint _vbo_vert_norm;
    unsigned int _num_indices;
    unsigned int _num_vertices;
    unsigned int _num_normals;
    unsigned int _num_faces;

    /* the global Assimp scene object */
    const C_STRUCT aiScene *scene;
    GLuint scene_list;

    float color[3];

    /* ---------------------------------------------------------------------------- */
    void color4_to_float4(const C_STRUCT aiColor4D *c, float f[4])
    {
        f[0] = c->r;
        f[1] = c->g;
        f[2] = c->b;
        f[3] = c->a;
    }

    /* ---------------------------------------------------------------------------- */
    void set_float4(float f[4], float a, float b, float c, float d)
    {
        f[0] = a;
        f[1] = b;
        f[2] = c;
        f[3] = d;
    }

    /* ---------------------------------------------------------------------------- */
    void apply_material(const C_STRUCT aiMaterial *mtl)
    {
        float c[4];

        GLenum fill_mode;
        int ret1, ret2;
        C_STRUCT aiColor4D diffuse;
        C_STRUCT aiColor4D specular;
        C_STRUCT aiColor4D ambient;
        C_STRUCT aiColor4D emission;
        ai_real shininess, strength;
        int two_sided;
        int wireframe;
        unsigned int max;

        set_float4(c, 0.8f, 0.8f, 0.8f, 1.0f);
        if (AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_DIFFUSE, &diffuse))
            color4_to_float4(&diffuse, c);
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, c);

        set_float4(c, 0.0f, 0.0f, 0.0f, 1.0f);
        if (AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_SPECULAR, &specular))
            color4_to_float4(&specular, c);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, c);

        set_float4(c, 0.2f, 0.2f, 0.2f, 1.0f);
        if (AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_AMBIENT, &ambient))
            color4_to_float4(&ambient, c);
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, c);

        set_float4(c, 0.0f, 0.0f, 0.0f, 1.0f);
        if (AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_EMISSIVE, &emission))
            color4_to_float4(&emission, c);
        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, c);

        max = 1;
        ret1 = aiGetMaterialFloatArray(mtl, AI_MATKEY_SHININESS, &shininess, &max);
        if (ret1 == AI_SUCCESS)
        {
            max = 1;
            ret2 = aiGetMaterialFloatArray(mtl, AI_MATKEY_SHININESS_STRENGTH, &strength, &max);
            if (ret2 == AI_SUCCESS)
                glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess * strength);
            else
                glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess);
        }
        else
        {
            glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 0.0f);
            set_float4(c, 0.0f, 0.0f, 0.0f, 0.0f);
            glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, c);
        }

        max = 1;
        if (AI_SUCCESS == aiGetMaterialIntegerArray(mtl, AI_MATKEY_ENABLE_WIREFRAME, &wireframe, &max))
            fill_mode = wireframe ? GL_LINE : GL_FILL;
        else
            fill_mode = GL_FILL;
        glPolygonMode(GL_FRONT_AND_BACK, fill_mode);

        max = 1;
        if ((AI_SUCCESS == aiGetMaterialIntegerArray(mtl, AI_MATKEY_TWOSIDED, &two_sided, &max)) && two_sided)
            glDisable(GL_CULL_FACE);
        else
            glEnable(GL_CULL_FACE);
    }
};
#endif
