import { useRef, useMemo } from 'react'
import { Canvas, useFrame } from '@react-three/fiber'
import * as THREE from 'three/webgpu'

function RainbowTriangle() {
  const meshRef = useRef<THREE.Mesh>(null!)

  const geometry = useMemo(() => {
    const geo = new THREE.BufferGeometry()
    const size = 1.5
    const h = (Math.sqrt(3) / 2) * size

    const vertices = new Float32Array([
       0,         h * 0.667,  0,
      -size / 2, -h * 0.333,  0,
       size / 2, -h * 0.333,  0,
    ])
    const colors = new Float32Array([
      1.0, 0.0, 0.0,  // 빨강
      0.0, 1.0, 0.0,  // 초록
      0.0, 0.0, 1.0,  // 파랑
    ])

    geo.setAttribute('position', new THREE.BufferAttribute(vertices, 3))
    geo.setAttribute('color',    new THREE.BufferAttribute(colors,    3))
    geo.setIndex([0, 1, 2])
    return geo
  }, [])

  useFrame((_, delta) => {
    meshRef.current.rotation.z -= delta * 0.8
  })

  return (
    <mesh ref={meshRef} geometry={geometry}>
      <meshBasicMaterial vertexColors side={THREE.DoubleSide} />
    </mesh>
  )
}

export default function App() {
  return (
    <div style={{
      width: '100vw', height: '100vh',
      background: '#0a0a0f',
      display: 'flex', flexDirection: 'column',
      alignItems: 'center', justifyContent: 'center',
    }}>
      <h1 style={{ color: '#fff', marginBottom: '1rem' }}>
        ⚡ WebGPU Rainbow Triangle
      </h1>
      <div style={{ width: 500, height: 500, borderRadius: '1rem', overflow: 'hidden' }}>
        <Canvas
  camera={{ position: [0, 0, 4], fov: 50 }}
  gl={async (props) => {
    const renderer = new THREE.WebGPURenderer(props)
    await renderer.init()
    console.log('✅ WebGPU OK!')
    return renderer as any
  }}
>
  <color attach="background" args={['#0a0a0f']} />
  <RainbowTriangle />
</Canvas>
      </div>
    </div>
  )
}