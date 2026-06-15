import * as THREE from 'three'

const scene = new THREE.Scene()

const camera = new THREE.PerspectiveCamera(
  75,
  window.innerWidth / window.innerHeight,
  0.1,
  1000
)

const renderer = new THREE.WebGLRenderer({
  antialias: true
})

renderer.setSize(
  window.innerWidth,
  window.innerHeight
)

document.body.appendChild(
  renderer.domElement
)

const geometry = new THREE.BufferGeometry()

const vertices = new Float32Array([
   0,  1, 0,
  -1, -1, 0,
   1, -1, 0
])

const colors = new Float32Array([
  1,0,0,
  0,1,0,
  0,0,1
])

geometry.setAttribute(
  'position',
  new THREE.BufferAttribute(vertices, 3)
)

geometry.setAttribute(
  'color',
  new THREE.BufferAttribute(colors, 3)
)

geometry.setIndex([0,1,2])

const material = new THREE.MeshBasicMaterial({
  vertexColors: true,
  side: THREE.DoubleSide
})

const triangle = new THREE.Mesh(
  geometry,
  material
)

scene.add(triangle)

camera.position.z = 3

function animate() {
  requestAnimationFrame(animate)

  triangle.rotation.z += 0.01

  renderer.render(scene, camera)
}

animate()