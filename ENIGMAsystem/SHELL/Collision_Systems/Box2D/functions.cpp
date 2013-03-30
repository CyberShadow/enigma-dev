/** Copyright (C) 2013 Robert B. Colton
***
*** This file is a part of the ENIGMA Development Environment.
***
*** ENIGMA is free software: you can redistribute it and/or modify it under the
*** terms of the GNU General Public License as published by the Free Software
*** Foundation, version 3 of the license or any later version.
***
*** This application and its source code is distributed AS-IS, WITHOUT ANY
*** WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
*** FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
*** details.
***
*** You should have received a copy of the GNU General Public License along
*** with this code. If not, see <http://www.gnu.org/licenses/>
**/
#include "Universal_System/collisions_object.h"
#include "Universal_System/instance_system.h" //iter
//#include "Universal_System/roomsystem.h"
#include "Collision_Systems/collision_mandatory.h" //iter
#include "Universal_System/instance.h"

#include <string>
#include <stdio.h>
#include <vector>
using std::vector;
//using namespace std;

#include <Box2D/Box2D.h>
#include "functions.h"
bool systemPaused = false;

static inline double r2d(double r) { return r * 180 / M_PI; }

// NOTES:
// 1) provide function overloads for interfacing with how studio is all fucked up so we don't have to limit ours
// 2) box2d uses an inversed y-axis, thus why physics_fixture_get_angle() returns -radianstodegrees(angle)
// 3) static objects when hit appear to me to move about one pixel at times, I can't tell if this is just my eyes playin tricks
// 4) worlds must be manually updated untill someone writes an update loop for this system which updates the current
//    rooms binded world, I suggest also leaving the option to continue allowing you to manually update your world
// 5) box2d's manual also states you should blend previous timesteps for updating the world with the current timestep
//    in order to make the simulation run smoother
// 6) box2d manual is available here... http://www.box2d.org/manual.html
// 7) I made joints bind fixtures, where as studio's joints bind instances together, that's stupid, fuck that
// 8) box2d's classes allow you to set a b2Shape for instance to a b2CircleShape or b2PolygonShape
//    that is why I wrote the classes to use an abstracted pointer reference such as b2Shape and b2Joint

struct worldInstance {
  b2World* world;
  // Prepare for simulation. Typically we use a time step of 1/60 of a
  // second (60Hz) and 10 iterations. This provides a high quality simulation
  // in most game scenarios.
  float32 timeStep;
  int32 velocityIterations;
  int32 positionIterations;
  int32 pixelstometers;
  bool paused;

  worldInstance()
  {
    // Define the gravity vector.
    b2Vec2 gravity(0.0f, 10.0f);

    // Construct a world object, which will hold and simulate the rigid bodies.
    world = new b2World(gravity);
    timeStep = 1.0f / 60.0f;
    velocityIterations = 8;
    pixelstometers = 32;
    paused = false;
  }

  void world_update();
}; 

void worldInstance::world_update() 
{
  if (!systemPaused && !paused) {
    world->Step(timeStep, velocityIterations, positionIterations);
    world->ClearForces();
  }
}

vector<worldInstance> worlds(1);

struct fixtureInstance {
  int world;
  b2Body* body;
  b2Fixture* fixture;
  b2Shape* shape;
  b2PolygonShape* polygonshape;
  vector<b2Vec2> vertices;

  fixtureInstance() 
  {

  }

  ~fixtureInstance()
  {

  }

  void FinishShape()
  {
    b2FixtureDef fixtureDef;
    fixtureDef.shape = shape;
    fixture = body->CreateFixture(&fixtureDef);
  }

}; 
vector<fixtureInstance> fixtures;

struct jointInstance {
  int world;
  b2Joint* joint;
  jointInstance()
  {
  }
}; 
vector<jointInstance> joints;

/* This is just place holder shit to get the fuckin linker shut the fuckin hell up */
enigma::instance_t collision_rectangle(double x1, double y1, double x2, double y2, int obj, bool prec, bool notme)
{

}

namespace enigma
{
  void *get_collision_mask(sprite* spr, unsigned char* input_data, collision_type ct) // It is called for every subimage of every sprite loaded.
  {
    return 0;
  }

  void free_collision_mask(void* mask)
  {
  }

  void perform_automatic_collision_handling_before_collision_event()
  {
    for (std::vector<worldInstance>::iterator it = worlds.begin(); it != worlds.end(); it++) {
      it->world_update();
    }
  }
};

bool place_free(double x, double y)
{
  for (int i = 0; i < fixtures.size(); i++)
  {
    if (fixtures[i].body->GetFixtureList()->TestPoint(b2Vec2(x, y))){
      return true;
    }
  }
  return false;
}

bool place_meeting(double x, double y, int object)
{
  return false;
}

/* This is where the actual physics_* functions start at */

void physics_world_create(int pixeltometerscale)
{
  // studio's fucked up world creation just auto binds it to the current room
  // thats fuckin retarded thus why i overloaded the function and provided an extra
  // function for setting the pixel to metre scale, because i refuse to write system their way
}

int physics_world_create()
{
  int i = worlds.size();
  worlds.resize(i + 1);
  worlds[i] = worldInstance();
  return i;
}

void physics_world_delete(int index)
{
 
}

void physics_world_pause_enable(int index, bool paused)
{
  if (unsigned(index) >= worlds.size() || index < 0)
  {
    return;
  }
  else
  {
    worlds[index].paused = paused;
  }
}

void physics_world_scale(int index, int pixelstometers)
{
  if (unsigned(index) >= worlds.size() || index < 0)
  {
    return;
  }
  else
  {
    worlds[index].pixelstometers = pixelstometers;
  }
}

void physics_world_gravity(int index, double gx, double gy)
{
  if (unsigned(index) >= worlds.size() || index < 0)
  {
    return;
  }
  else
  {
    worlds[index].world->SetGravity(b2Vec2(gx, gy));
  }
}

void physics_world_update(int index)
{
  // extra function to control your world update, should be auto done inside our game loop
  if (unsigned(index) >= worlds.size() || index < 0)
  {
    return;
  }
  else
  {
    worlds[index].world_update();
  }
}

void physics_world_update_settings(int index, double timeStep, int velocityIterations, int positionIterations)
{
  // extra function to control your world update settings
  if (unsigned(index) >= worlds.size() || index < 0)
  {
    return;
  }
  else
  {
    worlds[index].timeStep = timeStep;
    worlds[index].velocityIterations = velocityIterations;
    worlds[index].positionIterations = positionIterations;
  }
}

void physics_world_update_iterations(int iterationsperstep)
{
  // provide overloads if we do adopt this system so that you can still
  // change indexed worlds
}

void physics_world_update_iterations(int index, int iterationsperstep)
{
  // this sets the number of iterations the physics system takes each step
  // not needed for the current implementation
}

void physics_world_update_speed(int updatesperstep)
{
  // provide overloads if we do adopt this system so that you can still
  // change indexed worlds
}

void physics_world_update_speed(int index, int updatesperstep)
{
  // this sets the number of updates the physics system takes each step
  // not needed for the current implementation
}

void physics_world_draw_debug()
{
  // draws all the fixtures and their rotations in the room for u, wants constants, fuck that
  // end programmer can do it themselves
}

int physics_fixture_create(int world)
{
  if (unsigned(world) >= worlds.size() || world < 0)
  {
    return -1;
  }
  else
  {
    int i = fixtures.size();
    fixtures.resize(i + 1);
    fixtureInstance fixture = fixtureInstance();
    b2BodyDef bodyDef;
    bodyDef.type = b2_dynamicBody;
    fixture.body = worlds[world].world->CreateBody(&bodyDef);
    fixtures[i] = fixture;
    fixtures[i].world = world;
    return i;
  }
}

int physics_fixture_create()
{
  physics_fixture_create(0);
}

void physics_fixture_bind(int id)
{
  // binds a fixture to nothing, just closes and fills the definition
  if (unsigned(id) >= fixtures.size() || id < 0)
  {
    return;
  }
  else
  {

  }
}

void physics_fixture_bind()
{
  // binds a fixture to an object
}

void physics_fixture_set_collision_group(int id, int group)
{
  if (unsigned(id) >= fixtures.size() || id < 0)
  {
    return;
  }
  else
  {
    // sets the collision group used to make parts of things not collide, like a ragdoll for
    // instance should not collide with itself
    b2Filter newfilter;
    newfilter.groupIndex = group;
    fixtures[id].fixture->SetFilterData(newfilter);
  }
}

void physics_fixture_delete(int id)
{

}

void physics_fixture_set_box_shape(int id, double halfwidth, double halfheight)
{
  if (unsigned(id) >= fixtures.size() || id < 0)
  {
    return;
  }
  else
  {
    b2PolygonShape shape;
    shape.SetAsBox(halfwidth, halfheight);
    fixtures[id].shape = &shape;
    fixtures[id].FinishShape();
  }
}

void physics_fixture_set_circle_shape(int id, double radius)
{
  if (unsigned(id) >= fixtures.size() || id < 0)
  {
    return;
  }
  else
  {
    b2CircleShape shape;
    shape.m_radius = radius;
    fixtures[id].shape = &shape;
    fixtures[id].FinishShape();
  }
}

void physics_fixture_set_polygon_shape(int id)
{
  if (unsigned(id) >= fixtures.size() || id < 0)
  {
    return;
  }
  else
  {
    b2PolygonShape* shape;
    fixtures[id].polygonshape = shape;
    fixtures[id].FinishShape();
  }
}

void physics_fixture_add_point(int id, double x, double y)
{
  if (unsigned(id) >= fixtures.size() || id < 0)
  {
    return;
  }
  else
  {
    fixtures[id].vertices.push_back(b2Vec2(x, y));
    fixtures[id].polygonshape->Set(&fixtures[id].vertices[0], fixtures[id].vertices.size());
  }
}

void physics_fixture_set_transform(int id, double x, double y, double angle)
{
  if (unsigned(id) >= fixtures.size() || id < 0)
  {
    return;
  }
  else
  {
    fixtures[id].body->SetTransform(b2Vec2(x, y), angle);
  }
}

void physics_fixture_set_position(int id, double x, double y)
{
  if (unsigned(id) >= fixtures.size() || id < 0)
  {
    return;
  }
  else
  {
    fixtures[id].body->SetTransform(b2Vec2(x, y), fixtures[id].body->GetAngle());
  }
}

void physics_fixture_set_angle(int id, double angle)
{
  if (unsigned(id) >= fixtures.size() || id < 0)
  {
    return;
  }
  else
  {
    fixtures[id].body->SetTransform(fixtures[id].body->GetPosition(), angle);
  }
}

void physics_fixture_set_density(int id, double density)
{
  if (unsigned(id) >= fixtures.size() || id < 0)
  {
    return;
  }
  else
  {
    // technically studio makes it so 0 density, means infinite density and just makes it
    // a static object, but im just gonna go ahead and comment that out cause its fuckin stupid
    //if (density == 0) {
      //fixtures[id].body->SetType(b2_staticBody);
    //} else {
      fixtures[id].fixture->SetDensity(density);
      fixtures[id].body->ResetMassData();
    //}
  }
}

void physics_fixture_set_friction(int id, double friction)
{
  if (unsigned(id) >= fixtures.size() || id < 0)
  {
    return;
  }
  else
  {
    fixtures[id].fixture->SetFriction(friction);
    fixtures[id].body->ResetMassData();
  }
}

void physics_fixture_set_linear_damping(int id, double damping)
{
  if (unsigned(id) >= fixtures.size() || id < 0)
  {
    return;
  }
  else
  {
    fixtures[id].body->SetLinearDamping(damping);
  }
}

void physics_fixture_set_angular_damping(int id, double damping)
{
  if (unsigned(id) >= fixtures.size() || id < 0)
  {
    return;
  }
  else
  {
    fixtures[id].body->SetAngularDamping(damping);
  }
}

void physics_fixture_set_restitution(int id, double restitution)
{
  if (unsigned(id) >= fixtures.size() || id < 0)
  {
    return;
  }
  else
  {
    fixtures[id].fixture->SetRestitution(restitution);
  }
}

void physics_fixture_set_sensor(int id, bool state)
{
  if (unsigned(id) >= fixtures.size() || id < 0)
  {
    return;
  }
  else
  {
    fixtures[id].fixture->SetSensor(state);
  }
}

void physics_fixture_set_awake(int id, bool state)
{
  if (unsigned(id) >= fixtures.size() || id < 0)
  {
    return;
  }
  else
  {
    fixtures[id].body->SetAwake(state);
  }
}

void physics_fixture_set_sleep(int id, bool allowsleep)
{
  if (unsigned(id) >= fixtures.size() || id < 0)
  {
    return;
  }
  else
  {
    fixtures[id].body->SetSleepingAllowed(allowsleep);
  }
}

void physics_fixture_mass_properties(int id, double mass, double local_center_x, double local_center_y, double inertia)
{
  if (unsigned(id) >= fixtures.size() || id < 0)
  {
    return;
  }
  else
  {
     b2MassData lMassData;
     fixtures[id].body->GetMassData(&lMassData);
     lMassData.mass = mass;
     lMassData.center.Set(local_center_x, local_center_y);
     lMassData.I = inertia;
     fixtures[id].body->SetMassData(&lMassData);
  }
}

void physics_fixture_set_static(int id)
{
  if (unsigned(id) >= fixtures.size() || id < 0)
  {
    return;
  }
  else
  {
    fixtures[id].body->SetType(b2_staticBody);
    fixtures[id].body->ResetMassData();
  }
}

void physics_fixture_set_kinematic(int id)
{
  if (unsigned(id) >= fixtures.size() || id < 0)
  {
    return;
  }
  else
  {
    fixtures[id].body->SetType(b2_kinematicBody);
  }
}

void physics_fixture_set_dynamic(int id)
{
  if (unsigned(id) >= fixtures.size() || id < 0)
  {
    return;
  }
  else
  {
    fixtures[id].body->SetType(b2_dynamicBody);
  }
}

double physics_fixture_get_angle(int id)
{
  if (unsigned(id) >= fixtures.size() || id < 0)
  {
    return 0;
  }
  else
  {
    return -r2d(fixtures[id].body->GetAngle());
  }
}

double physics_fixture_get_x(int id)
{
  if (unsigned(id) >= fixtures.size() || id < 0)
  {
    return 0;
  }
  else
  {
    return fixtures[id].body->GetPosition().x;
  }
}

double physics_fixture_get_y(int id)
{
  if (unsigned(id) >= fixtures.size() || id < 0)
  {
    return 0;
  }
  else
  {
    return fixtures[id].body->GetPosition().y;
  }
}

double physics_fixture_get_mass(int id)
{
  if (unsigned(id) >= fixtures.size() || id < 0)
  {
    return 0;
  }
  else
  {
     b2MassData lMassData;
     fixtures[id].body->GetMassData(&lMassData);
     return lMassData.mass;
  }
}

double physics_fixture_get_center_x(int id)
{
  if (unsigned(id) >= fixtures.size() || id < 0)
  {
    return 0;
  }
  else
  {
     b2MassData lMassData;
     fixtures[id].body->GetMassData(&lMassData);
     return lMassData.center.x;
  }
}

double physics_fixture_get_center_y(int id)
{
  if (unsigned(id) >= fixtures.size() || id < 0)
  {
    return 0;
  }
  else
  {
     b2MassData lMassData;
     fixtures[id].body->GetMassData(&lMassData);
     return lMassData.center.y;
  }
}

double physics_fixture_get_inertia(int id)
{
  if (unsigned(id) >= fixtures.size() || id < 0)
  {
    return 0;
  }
  else
  {
     b2MassData lMassData;
     fixtures[id].body->GetMassData(&lMassData);
     return lMassData.I;
  }
}

void physics_apply_force(int world, double xpos, double ypos, double xforce, double yforce)
{
  if (unsigned(world) >= worlds.size() || world < 0)
  {
    return;
  }
  else
  {
    for (int i = 0; i < fixtures.size(); i++)
    {
      if (fixtures[i].world == world)
      {
        fixtures[i].body->ApplyForce(b2Vec2(xforce, yforce), b2Vec2(xpos, ypos));
      }
    }
  }
}

void physics_apply_impulse(int world, double xpos, double ypos, double ximpulse, double yimpulse)
{
  if (unsigned(world) >= worlds.size() || world < 0)
  {
    return;
  }
  else
  {
    for (int i = 0; i < fixtures.size(); i++)
    {
      if (fixtures[i].world == world)
      {
        fixtures[i].body->ApplyLinearImpulse(b2Vec2(ximpulse, yimpulse), b2Vec2(xpos, ypos));
      }
    }
  }
}

void physics_apply_local_force(int id, double xlocal, double ylocal, double xforce, double yforce)
{
  if (unsigned(id) >= fixtures.size() || id < 0)
  {
    return;
  }
  else
  {
    fixtures[id].body->ApplyForce(b2Vec2(xforce, yforce), b2Vec2(xlocal, ylocal));
  }
}

void physics_apply_local_impulse(int id, double xlocal, double ylocal, double ximpulse, double yimpulse)
{
  if (unsigned(id) >= fixtures.size() || id < 0)
  {
    return;
  }
  else
  {
    fixtures[id].body->ApplyLinearImpulse(b2Vec2(ximpulse, yimpulse), b2Vec2(xlocal, ylocal));
  }
}

void physics_apply_local_torque(int id, double torque)
{
  if (unsigned(id) >= fixtures.size() || id < 0)
  {
    return;
  }
  else
  {
    fixtures[id].body->ApplyTorque(torque);
  }
}

int physics_joint_create(int world)
{
    int i = joints.size();
    joints.resize(i + 1);
    jointInstance joint = jointInstance();
    joint.world = world;
    joints[i] = joint;
    return i;
}

void physics_joint_distance_create(int id, int fixture1, int fixture2)
{
  if (unsigned(id) >= joints.size() || id < 0)
  {
    return;
  }
  else
  {
    b2JointDef jointDef;
    jointDef.bodyA = fixtures[fixture1].body;
    jointDef.bodyB = fixtures[fixture2].body;
    //jointDef.target.Set(350, 320);
    //jointDef.maxForce = 3000.0 * body.m_mass;
    //jointDef.timeStep = m_timeStep;
    joints[id].joint = worlds[joints[id].world].world->CreateJoint(&jointDef);
  }
}

void physics_joint_mouse_create(int id, int fixture)
{
  if (unsigned(id) >= joints.size() || id < 0)
  {
    return;
  }
  else
  {
    b2MouseJointDef jointDef;
    //jointDef.body1 = m_world.m_groundBody;
    //jointDef.body2 = body;
    //jointDef.target.Set(350, 320);
    //jointDef.maxForce = 3000.0 * body.m_mass;
    //jointDef.timeStep = m_timeStep;
    joints[id].joint = worlds[joints[id].world].world->CreateJoint(&jointDef);
  }
}

void physics_joint_set_target(int id, double x, double y)
{
  if (unsigned(id) >= joints.size() || id < 0)
  {
    return;
  }
  else
  {
    //joints[id].joint->SetTarget(b2Vec2(x, y));
  }
}


void physics_pause_enable(bool pause)
{
  systemPaused = pause;
}

void physics_mass_properties(double mass, double local_center_x, double local_center_y, double inertia)
{
  // same as physics_fixture_mass_properties except it doesnt need an id, uses the currently bound fixture
  // of whatever is calling the function, im not writing it cause its stupid
}

void physics_draw_debug()
{
  // draws the currently bound fixture's shape with draw color, fuck that let the end programmer have control
}
