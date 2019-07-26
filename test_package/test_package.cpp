#include <Eigen/Dense>
#include <fcl/math/constants.h>
#include <fcl/narrowphase/collision.h>
#include <fcl/narrowphase/collision_object.h>
#include <functional>
#include <map>
#include <string>
#include <utility>
#include <vector>

using fcl::AngleAxis;
using fcl::Transform3;
using fcl::Vector3;
using std::map;
using std::pair;
using std::string;
using std::vector;

// Simple specification for defining a box collision object. Specifies the
// dimensions and pose of the box in some frame F (X_FB). For an explanation
// of the notation X_FB, see:
// http://drake.mit.edu/doxygen_cxx/group__multibody__spatial__pose.html
template <typename S> struct BoxSpecification {
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
  Vector3<S> size;
  Transform3<S> X_FB;
};

int main(int argc, char **argv) {
  const double pi = fcl::constants<double>::pi();
  const double size_1 = 1;
  BoxSpecification<double> box_spec_1{
      Vector3<double>{size_1, size_1, size_1},
      Transform3<double>{
          AngleAxis<double>(pi / 4, fcl::Vector3<double>::UnitY())}};

  const double size_2 = 3;
  BoxSpecification<double> box_spec_2{
      fcl::Vector3<double>{size_2, size_2, size_2},
      fcl::Transform3<double>{
          fcl::Translation3<double>(fcl::Vector3<double>(0, 0, -size_2 / 2))}};

  fcl::Vector3<double> expected_normal{0, 0, -1};
  double expected_depth = size_1 * sqrt(2) / 2;

  // Initialize isomorphic rotations of box 2.
  map<string, fcl::Transform3<double>, std::less<string>,
      Eigen::aligned_allocator<
          std::pair<const string, fcl::Transform3<double>>>>
      iso_poses;
  iso_poses["top"] = Transform3<double>::Identity();
  iso_poses["bottom"] =
      Transform3<double>{AngleAxis<double>(pi, Vector3<double>::UnitX())};
  iso_poses["back"] =
      Transform3<double>{AngleAxis<double>(pi / 2, Vector3<double>::UnitX())};
  iso_poses["front"] = Transform3<double>{
      AngleAxis<double>(3 * pi / 2, Vector3<double>::UnitX())};
  iso_poses["left"] =
      Transform3<double>{AngleAxis<double>(pi / 2, Vector3<double>::UnitY())};
  iso_poses["right"] = Transform3<double>{
      AngleAxis<double>(3 * pi / 2, Vector3<double>::UnitY())};

  fcl::Contact<double> expected_contact;
  expected_contact.penetration_depth = expected_depth;

  for (const auto &reorient_pair : iso_poses) {
    const std::string &top_face = reorient_pair.first;
    const fcl::Transform3<double> &pre_pose = reorient_pair.second;

    BoxSpecification<double> box_2_posed{box_spec_2.size,
                                         box_spec_2.X_FB * pre_pose};

    // Collide (1, 2)
    expected_contact.normal = expected_normal;
    {
      using CollisionGeometryPtr_t =
          std::shared_ptr<fcl::CollisionGeometry<double>>;
      CollisionGeometryPtr_t box_geometry_A(
          new fcl::Box<double>(box_spec_1.size));
      CollisionGeometryPtr_t box_geometry_B(
          new fcl::Box<double>(box_spec_2.size));

      fcl::CollisionObject<double> box_A(box_geometry_A, box_spec_1.X_FB);
      fcl::CollisionObject<double> box_B(box_geometry_B, box_spec_2.X_FB);

      // Compute collision - single contact and enable contact.
      fcl::CollisionRequest<double> collisionRequest(1, true);
      collisionRequest.gjk_solver_type = fcl::GST_LIBCCD;
      fcl::CollisionResult<double> collisionResult;
      fcl::collide(&box_A, &box_B, collisionRequest, collisionResult);
      std::vector<fcl::Contact<double>> contacts;
      collisionResult.getContacts(contacts);

      const fcl::Contact<double> &contact = contacts[0];
    }

    // Collide (2, 1)
    expected_contact.normal = -expected_normal;
    {
      using CollisionGeometryPtr_t =
          std::shared_ptr<fcl::CollisionGeometry<double>>;
      CollisionGeometryPtr_t box_geometry_A(
          new fcl::Box<double>(box_spec_2.size));
      CollisionGeometryPtr_t box_geometry_B(
          new fcl::Box<double>(box_spec_1.size));

      fcl::CollisionObject<double> box_A(box_geometry_A, box_spec_2.X_FB);
      fcl::CollisionObject<double> box_B(box_geometry_B, box_spec_1.X_FB);

      // Compute collision - single contact and enable contact.
      fcl::CollisionRequest<double> collisionRequest(1, true);
      collisionRequest.gjk_solver_type = fcl::GST_LIBCCD;
      fcl::CollisionResult<double> collisionResult;
      fcl::collide(&box_A, &box_B, collisionRequest, collisionResult);
      std::vector<fcl::Contact<double>> contacts;
      collisionResult.getContacts(contacts);

      const fcl::Contact<double> &contact = contacts[0];
    }
    
  }
}
