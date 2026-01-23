let PI = 3.141592653589793
let SOLARMASS = 4.0 * PI * PI
let DAYSPERYEAR = 365.24

func sqrt(float x) float {
    let guess = x / 2.0
    let epsilon = 0.00001

    for (let i = 0; i < 50; i = i + 1) {
        let next = (guess + x / guess) / 2.0
        if (guess - next < epsilon && next - guess < epsilon) {
            return next
        }
        guess = next
    }

    return guess
}

struct Body {
    x: float
    y: float
    z: float
    vx: float
    vy: float
    vz: float
    mass: float

    func offsetMomentum(float px, float py, float pz){
        vx = -px / SOLARMASS
        vy = -py / SOLARMASS
        vz = -pz / SOLARMASS
    }
}

func Jupiter() Body {
    return new Body(4.84143144246472090, -1.16032004402742839, -0.103622044471123109, 0.00166007664274403694 * DAYSPERYEAR, 0.00769901118419740425 * DAYSPERYEAR, -0.0000690460016972063023 * DAYSPERYEAR, 0.000954791938424326609 * SOLARMASS)
}

func Saturn() Body {
    return new Body(8.34336671824457987, 4.12479856412430479, -0.403523417114321381, -0.00276742510726862411 * DAYSPERYEAR, 0.00499852801234917238 * DAYSPERYEAR, 0.0000230417297573763929 * DAYSPERYEAR, 0.000285885980666130812 * SOLARMASS)
}

func Uranus() Body {
    return new Body(12.8943695621391310, -15.1111514016986312, -0.223307578892655734, 0.00296460137564761618 * DAYSPERYEAR, 0.00237847173959480950 * DAYSPERYEAR, -0.0000296589568540237556 * DAYSPERYEAR, 0.0000436624404335156298 * SOLARMASS)
}

func Neptune() Body {
    return new Body(15.3796971148509165, -25.9193146099879641, 0.179258772950371181, 0.00268067772490389322 * DAYSPERYEAR, 0.00162824170038242295 * DAYSPERYEAR, -0.0000951592254519715870 * DAYSPERYEAR, 0.0000515138902046611451 * SOLARMASS)
}

func Sun() Body {
    return new Body(0.0, 0.0, 0.0, 0.0, 0.0, 0.0, SOLARMASS)
}

struct NBodySystem {
    bodies: []Body

    func advance(float dt) {
        let size = len(bodies)

        for (let i = 0; i < size; i = i + 1) {
            let bodyi = bodies[i]

            for (let j = i + 1; j < size; j = j + 1) {
                let bodyj = bodies[j]

                let dx = bodyi.x - bodyj.x
                let dy = bodyi.y - bodyj.y
                let dz = bodyi.z - bodyj.z

                let distance = sqrt(dx * dx + dy * dy + dz * dz)
                let mag = dt / (distance * distance * distance)

                bodyi.vx = bodyi.vx - dx * bodyj.mass * mag
                bodyi.vy = bodyi.vy - dy * bodyj.mass * mag
                bodyi.vz = bodyi.vz - dz * bodyj.mass * mag

                bodyj.vx = bodyj.vx + dx * bodyi.mass * mag
                bodyj.vy = bodyj.vy + dy * bodyi.mass * mag
                bodyj.vz = bodyj.vz + dz * bodyi.mass * mag

                bodies[i] = bodyi
                bodies[j] = bodyj
            }
        }

        for (let i = 0; i < size; i = i + 1) {
            let body = bodies[i]
            body.x = body.x + dt * body.vx
            body.y = body.y + dt * body.vy
            body.z = body.z + dt * body.vz
            bodies[i] = body
        }
    }

    func energy() float {
        let e = 0.0
        let size = len(bodies)

        for (let i = 0; i < size; i = i + 1) {
            let bodyi = bodies[i]

            e = e + 0.5 * bodyi.mass * (bodyi.vx * bodyi.vx + bodyi.vy * bodyi.vy + bodyi.vz * bodyi.vz)

            for (let j = i + 1; j < size; j = j + 1) {
                let bodyj = bodies[j]

                let dx = bodyi.x - bodyj.x
                let dy = bodyi.y - bodyj.y
                let dz = bodyi.z - bodyj.z

                let distance = sqrt(dx * dx + dy * dy + dz * dz)

                e = e - (bodyi.mass * bodyj.mass) / distance
            }
        }

        return e
    }
}

func NBodySystemNew([]Body bodies) NBodySystem {
    let px = 0.0
    let py = 0.0
    let pz = 0.0
    let size = len(bodies)

    for (let i = 0; i < size; i = i + 1) {
        let b = bodies[i]
        let m = b.mass
        px = px + b.vx * m
        py = py + b.vy * m
        pz = pz + b.vz * m
    }

    bodies[0].offsetMomentum(px, py, pz)

    return new NBodySystem(bodies)
}

let ret = 0.0

let n = 3
while (n < 25) {
    let bodies = []Body(5)
    bodies[0] = Sun()
    bodies[1] = Jupiter()
    bodies[2] = Saturn()
    bodies[3] = Uranus()
    bodies[4] = Neptune()

    let system = NBodySystemNew(bodies)
    let max = n * 100

    ret = ret + system.energy()

    for (let i = 0; i < max; i = i + 1) {
        system.advance(0.01)
    }

    ret = ret + system.energy()

    n = n * 2
}

let expected = -1.3524862408537381
print(expected)
print(ret)
