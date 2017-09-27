#include "CCfunctions.hpp"
#include "MATHfunctions.hpp"
#include "HFfunctions.hpp"
#include "INTfunctions.hpp"
#include "TESTfunctions.hpp"
#include "BASISfunctions.hpp"
#include "OPfunctions.hpp"

int main(int argc, char * argv[])
{
  // basis = "infinite", "finite", "finite_J"
  // case = "nuclear", "electronic"
  // approx = "doubles", "singles", "triples"
  std::srand(time(NULL));
  struct timespec time1, time2;
  double elapsed0 = 0.0;

  Input_Parameters Parameters;
  Model_Space Space;
  Channels Chan;
  Amplitudes Amps;
  Interactions Ints;
  Eff_Interactions Eff_Ints;
  HF_Channels HF_Chan;
  HF_Matrix_Elements HF_ME;
  Single_Particle_States States;
  double Energy, Energy0;
  int JM = 0;
  
  Parameters.extra = 100;
  clock_gettime(CLOCK_MONOTONIC, &time1);
  if(argc == 1 || argc == 2){
    std::string inputfile;
    if(argc == 1){ inputfile = "input.dat"; }
    else{ inputfile = argv[1]; }

    Get_Input_Parameters(inputfile, Parameters);
    if(Parameters.basis == "infinite"){ Parameters.approx == "doubles"; }
    Parameters.HF = 1;
    //Parameters.basis = "finite_JM";

    Build_Model_Space(Parameters, Space);
    Print_Parameters(Parameters, Space);
    HF_Chan = HF_Channels(Parameters, Space);
    States = Single_Particle_States(Parameters, Space, HF_Chan);
    if(Parameters.basis == "finite_J" || Parameters.basis == "finite_JM"){ Read_Matrix_Elements_J(Parameters, Space, HF_Chan, HF_ME); }
    else{ Read_Matrix_Elements_M(Parameters, Space, HF_Chan, HF_ME); }

    if(Parameters.HF == 1){
      Hartree_Fock_States(Parameters, Space, HF_Chan, States, HF_ME);
      Convert_To_HF_Matrix_Elements(Parameters, HF_Chan, Space, States, HF_ME);
    }
    if(Parameters.basis == "finite_JM"){
      JM = 1;
      Parameters.basis = "finite_M";
      Build_Model_Space_J2(Parameters, Space);
      Print_Parameters(Parameters, Space);
    }
    Chan = Channels(Parameters, Space);
    Amps = Amplitudes(Parameters, Space, Chan);
    Ints = Interactions(Parameters, Chan);
    
    if(Parameters.HF == 0){ Get_Fock_Matrix(Parameters, HF_Chan, HF_ME, States, Space, Chan, Ints); }
    Eff_Ints = Eff_Interactions(Parameters, Space, Chan);
    if(Parameters.basis == "finite_J"){ Get_Matrix_Elements_J(Parameters, HF_Chan, HF_ME, Space, Chan, Ints); }
    else if(JM == 1){ Get_Matrix_Elements_JM(Parameters, HF_Chan, HF_ME, Space, Chan, Ints); }
    else{ Get_Matrix_Elements(Parameters, HF_Chan, HF_ME, Space, Chan, Ints); }
    States.delete_struct(HF_Chan);
    HF_ME.delete_struct(HF_Chan);
    HF_Chan.delete_struct();
  }
  else if(argc == 6 || argc == 7){
    Parameters.calc_case = argv[1];
    Parameters.density = atof(argv[2]);
    Parameters.Shells = atoi(argv[3]);
    Parameters.Pshells = atoi(argv[4]);
    Parameters.Nshells = atoi(argv[5]);
    Parameters.obstrength = 1.0;
    Parameters.tbstrength = 1.0;
    if(argc == 7){ Parameters.extra = atoi(argv[6]); }
    
    if(Parameters.calc_case == "nuclear"){
      Parameters.basis = "infinite";
      Parameters.approx = "doubles";
      CART_Build_Model_Space(Parameters, Space);
      Print_Parameters(Parameters, Space);
      Chan = Channels(Parameters, Space);
      Amps = Amplitudes(Parameters, Space, Chan);
      Ints = Interactions(Parameters, Chan);
      Eff_Ints = Eff_Interactions(Parameters, Space, Chan);
      Minnesota_Matrix_Elements(Parameters, Space, Chan, Ints);
    }
    else if(Parameters.calc_case == "electronic"){
      Parameters.basis = "infinite";
      Parameters.approx = "doubles";
      CART_Build_Model_Space(Parameters, Space);
      Print_Parameters(Parameters, Space);
      Chan = Channels(Parameters, Space);
      Amps = Amplitudes(Parameters, Space, Chan);
      Ints = Interactions(Parameters, Chan);
      Eff_Ints = Eff_Interactions(Parameters, Space, Chan);
      Coulomb_Inf_Matrix_Elements(Parameters, Space, Chan, Ints);
    }
    else if(Parameters.calc_case == "quantum_dot"){
      Parameters.HF = 1;
      Parameters.basis = "finite_HO";
      Parameters.approx = "singles";
      QD_Build_Model_Space(Parameters, Space);
      Print_Parameters(Parameters, Space);
      HF_Chan = HF_Channels(Parameters, Space);
      States = Single_Particle_States(Parameters, Space, HF_Chan);
      //Read_Matrix_Elements_QD(Parameters, Space, HF_Chan, HF_ME);
      Read_QD_ME_From_File(Parameters, Space, HF_Chan, HF_ME);

      if(Parameters.HF == 1){
	Hartree_Fock_States(Parameters, Space, HF_Chan, States, HF_ME);
	Convert_To_HF_Matrix_Elements(Parameters, HF_Chan, Space, States, HF_ME);
      }
      Chan = Channels(Parameters, Space);
      Amps = Amplitudes(Parameters, Space, Chan);
      Ints = Interactions(Parameters, Chan);

      if(Parameters.HF == 0){
	Get_Fock_Matrix(Parameters, HF_Chan, HF_ME, States, Space, Chan, Ints);
      }
      Eff_Ints = Eff_Interactions(Parameters, Space, Chan);
      Get_Matrix_Elements(Parameters, HF_Chan, HF_ME, Space, Chan, Ints);
      States.delete_struct(HF_Chan);
      HF_ME.delete_struct(HF_Chan);
      HF_Chan.delete_struct();
    }
  }

  //Perform_CC_Test(Parameters, Space, Chan, Ints, Eff_Ints, Amps);
  Perform_CC(Parameters, Space, Chan, Ints, Eff_Ints, Amps);
  Energy = E_Ref(Parameters, Space, Chan, Ints);
  Energy0 = Amps.get_energy(Parameters, Space, Chan, Ints);
  std::cout << std::setprecision(10);
  if(Parameters.approx == "singles"){
    std::cout << "Eref = " << Energy << ", dCCSD = " << Energy0 << std::endl;
    std::cout << "Eref/A = " << Energy/(Parameters.P + Parameters.N) << ", dCCSD/A = " << Energy0/(Parameters.P + Parameters.N) << std::endl;
  }
  else{
    std::cout << "Eref = " << Energy << ", dCCD = " << Energy0 << std::endl;
    std::cout << "Eref/A = " << Energy/(Parameters.P + Parameters.N) << ", dCCD/A = " << Energy0/(Parameters.P + Parameters.N) << std::endl;
  }      
  Energy += Energy0;
  std::cout << "E = " << Energy << ", E/A = " << Energy/(Parameters.P + Parameters.N) << std::endl << std::endl;

  if(Parameters.extra == 1){
    EOM EOM_1PA, EOM_1PR;
    Update_Heff_3(Parameters, Space, Chan, Ints, Eff_Ints, Amps);
    std::cout << "1PA-EOM" << std::endl;
    EOM_1PA.PA1_EOM(Parameters, Space, Chan, Eff_Ints);
    EOM_1PA.Print_EOM_1P(Parameters, Energy);
    std::cout << "1PR-EOM" << std::endl;
    EOM_1PR.PR1_EOM(Parameters, Space, Chan, Eff_Ints);
    EOM_1PR.Print_EOM_1P(Parameters, Energy);

    GT GT1 = GT(Parameters, Space, Chan, Amps);

    EOM_1PA.delete_struct();
    EOM_1PR.delete_struct();
    GT1.delete_struct();
  }

  Ints.delete_struct(Parameters, Chan);
  Eff_Ints.delete_struct(Parameters, Chan);
  Amps.delete_struct(Parameters, Chan);
  Chan.delete_struct(Parameters);
  Space.delete_struct(Parameters);

  clock_gettime(CLOCK_MONOTONIC, &time2);
  elapsed0 = (time2.tv_sec - time1.tv_sec);
  elapsed0 += (time2.tv_nsec - time1.tv_nsec) / 1000000000.0;
  std::cout << std::endl << "Total Runtime = " << elapsed0 << " sec. " << std::endl;

  return 0;
}
