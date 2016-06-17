#version 120

varying vec2 TexCoord;

void main(void) {
    TexCoord = gl_MultiTexCoord0.st;
    //gl_Position = ftransform();
    gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix * gl_Vertex;
}
