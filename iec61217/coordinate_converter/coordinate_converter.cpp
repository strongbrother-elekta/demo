// coordinate_converter.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <Eigen/Dense>
#include <unsupported/Eigen/NonLinearOptimization>

#ifdef M_PI
#undef M_PI
#endif
#define M_PI  3.1415926

enum class RotationSequence
{
    XYZ,
    ZYZ,
    ZYX,
};

template <typename Scalar>
class PpsMath
{
public:
    typedef Eigen::Matrix<Scalar, 3, 3> MatrixR;
    typedef Eigen::Matrix<Scalar, 4, 4> TransformMatrix;
    typedef Eigen::Matrix<Scalar, 3, 1> Euler;
    typedef Eigen::Matrix<Scalar, 4, 1> VectorH;
    typedef Eigen::Matrix<Scalar, 3, 1> Translation;
    typedef Eigen::AngleAxis<Scalar> AngleAxis;

    /**
     * Converts the given Euler angles from degrees to radians.
     *
     * @param deg The Euler angles in degrees.
     * @return The Euler angles converted to radians.
     */
    static Euler deg2RadV(Euler deg)
    {
        return deg * M_PI / 180;
    }

    /**
     * Converts the given Euler angle from radians to degrees.
     *
     * @param rad The Euler angle in radians.
     * @return The Euler angle in degrees.
     */
    static Euler rad2DegV(Euler rad)
    {
        return rad * 180 / M_PI;
    }

    /**
     * \brief converts a set of Euler angles, eul, to the corresponding rotation matrix, rotm. When using the rotation
     matrix, premultiply it with the coordinates to be rotated (as opposed to postmultiplying). The default order for
     Euler angle rotations is "XYZ".
     * \param euler Euler rotation angles in degree
     * \param sequence Axis rotation sequence for the Euler angles, specified as one of these string scalars:

              "ZYX" (default) The order of rotation angles is z-axis, y-axis, x-axis.

              "ZYZ" The order of rotation angles is z-axis, y-axis, z-axis.

              "XYZ" The order of rotation angles is x-axis, y-axis, z-axis.
     * \return Rotation matrix, returned as a 3-by-3 matrix containing rotation matrices.
     */
    static MatrixR eul2rotm(Euler euler, RotationSequence sequence = RotationSequence::XYZ)
    {
        MatrixR R = MatrixR::Identity();
        euler = deg2RadV(euler);

        switch (sequence)
        {
        case RotationSequence::XYZ:
        {
            auto mx = AngleAxis(euler(0), Euler::UnitX());
            auto my = AngleAxis(euler(1), Euler::UnitY());
            auto mz = AngleAxis(euler(2), Euler::UnitZ());
            R = mx * my * mz;
        }
        break;
        case RotationSequence::ZYZ:
        {
            auto mz1 = AngleAxis(euler(0), Euler::UnitZ());
            auto my = AngleAxis(euler(1), Euler::UnitY());
            auto mz2 = AngleAxis(euler(2), Euler::UnitZ());
            R = mz1 * my * mz2;
        }
        break;
        case RotationSequence::ZYX:
        {
            auto mz = AngleAxis(euler(0), Euler::UnitZ());
            auto my = AngleAxis(euler(1), Euler::UnitY());
            auto mx = AngleAxis(euler(2), Euler::UnitX());
            R = mz * my * mx;
        }
        break;
        default:
            break;
        }
        return R;
    }

    /**
     * \brief converts a rotation matrix, matrix, to the corresponding Euler angles, eul. The input rotation matrix must
     be in the premultiply form for rotations. The default order for Euler angle rotations is "ZYX".
     * \param matrix Rotation matrix, specified as a 3-by-3 matrix containing rotation matrices. Each rotation matrix has
     a size of 3-by-3 and is orthonormal.
     * \param sequence  Axis rotation sequence for the Euler angles, specified as one of these string scalars:

              "ZYX" (default) The order of rotation angles is z-axis, y-axis, x-axis.

              "ZYZ" The order of rotation angles is z-axis, y-axis, z-axis.

              "XYZ" The order of rotation angles is x-axis, y-axis, z-axis.
     * \return euler Euler rotation angles in degree
     */
    static Euler rotm2Eul(MatrixR matrix, RotationSequence sequence = RotationSequence::XYZ)
    {
        Euler r(0, 0, 0);
        switch (sequence)
        {
        case RotationSequence::XYZ:
            r = matrix.eulerAngles(0, 1, 2);
            break;
        case RotationSequence::ZYZ:
            r = matrix.eulerAngles(2, 1, 2);
            break;
        case RotationSequence::ZYX:
            r = matrix.eulerAngles(2, 1, 0);
            break;
        default:
            break;
        }
        return rad2DegV(r);
    }

    /**
     * @brief Converts Euler angles to a homogeneous transformation matrix.
     *
     * This function takes Euler angles and a rotation sequence as input and returns a homogeneous transformation matrix.
     * The rotation sequence determines the order in which the rotations are applied.
     *
     * @param euler The Euler angles to be converted.
     * @param sequence The rotation sequence to be used (default: XYZ).
     * @return The resulting homogeneous transformation matrix.
     */
    static TransformMatrix eul2Homom(Euler euler, RotationSequence sequence = RotationSequence::XYZ)
    {
        TransformMatrix r = TransformMatrix::Identity();
        r.topLeftCorner(3, 3) = eul2rotm(euler, sequence);
        return r;
    }

    /**
     * Converts Euler angles to a homogeneous transformation matrix.
     *
     * @param r1 The first rotation angle.
     * @param r2 The second rotation angle.
     * @param r3 The third rotation angle.
     * @param sequence The rotation sequence to use (default is XYZ).
     * @return The resulting homogeneous transformation matrix.
     */
    static TransformMatrix eul2Homom(Scalar r1, Scalar r2, Scalar r3, RotationSequence sequence = RotationSequence::XYZ)
    {
        Euler euler(r1, r2, r3);
        return eul2Homom(euler, sequence);
    }

    static TransformMatrix tra2Homom(Translation tra)
    {
        TransformMatrix r = TransformMatrix::Identity();
        r.topRightCorner(3, 1) = tra;
        return r;
    }

    /**
     * Converts the given translation coordinates to a homogeneous transformation matrix.
     *
     * @param x The x-coordinate of the translation.
     * @param y The y-coordinate of the translation.
     * @param z The z-coordinate of the translation.
     * @return The resulting homogeneous transformation matrix.
     */
    static TransformMatrix tra2Homom(Scalar x, Scalar y, Scalar z)
    {
        Translation tra(x, y, z);

        return tra2Homom(tra);
    }

    /**
     * @brief Creates a shift transformation matrix.
     *
     * This function creates a shift transformation matrix based on the given translation and rotation parameters.
     *
     * @param x The translation along the x-axis.
     * @param y The translation along the y-axis.
     * @param z The translation along the z-axis.
     * @param pitch The rotation angle around the x-axis.
     * @param roll The rotation angle around the y-axis.
     * @param yaw The rotation angle around the z-axis.
     * @param sequence The rotation sequence to apply (default: XYZ).
     *
     * @return The resulting shift transformation matrix.
     */
    static TransformMatrix createShift(Scalar x,
        Scalar y,
        Scalar z,
        Scalar pitch,
        Scalar roll,
        Scalar yaw,
        RotationSequence sequence = RotationSequence::XYZ)
    {
        Euler euler(
            pitch,
            yaw,
            roll); //lint !e529  local variable 'euler' declared in 'PpsMath::createShift' not subsequently referenced

        TransformMatrix r = eul2Homom(pitch, roll, yaw, sequence);
        TransformMatrix t = tra2Homom(x, y, z);
        return t * r;
    }
};

int main()
{
    std::cout << "Hello World!\n";
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
