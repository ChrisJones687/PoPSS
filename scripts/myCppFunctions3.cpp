#include <Rcpp.h>
#include <omp.h>
using namespace Rcpp;
// [[Rcpp::plugins(openmp)]]

//Within each infected cell (I > 0) draw random number of infections ~Poisson(lambda=rate of spore production) for each infected host. 
//Take SUM for total infections produced by each cell. 

// [[Rcpp::export]]
IntegerMatrix SporeGenCpp(IntegerMatrix infected, NumericMatrix weather_suitability, double rate){
  
  // internal variables
  int nrow = infected.nrow(); 
  int ncol = infected.ncol();
  
  IntegerMatrix SP = clone<IntegerMatrix>(infected);
  //Function rpois("rpois");
  
  RNGScope scope;
  
  // LOOP THROUGH EACH INFECTED CELL AND GENERATE AMOUNT OF SPORES
  //#pragma omp parallel for
  for (int row = 0; row < nrow; row++) {
    for (int col = 0; col < ncol; col++){
      
      if (infected(row, col) > 0){  //if infected > 0, generate spores proportional to production rate * weather suitability
        double lambda = rate * weather_suitability(row, col);
        NumericVector inf = rpois(infected(row, col), lambda);
        int s = sum(inf);
        SP(row, col) = s;
      }
    }
  }
  return SP;
}

// [[Rcpp::export]]
List SporeDispCpp_MH(IntegerMatrix spore_matrix, 
                     IntegerMatrix S_host1_mat, IntegerMatrix S_LD, IntegerMatrix S_host2_mat, 
                     IntegerMatrix I_host1_mat, IntegerMatrix I_LD, IntegerMatrix I_host2_mat, 
                     IntegerMatrix N_LVE,
                     NumericMatrix weather_suitability,   //use different name than the functions in myfunctions_SOD.r
                     double rs, String rtype, double scale1, 
                     double scale2=NA_REAL,  //default values
                     double gamma=NA_REAL)	//default values
{  
  
  // internal variables //
  int nrow = spore_matrix.nrow(); 
  int ncol = spore_matrix.ncol();
  int row0;
  int col0;
  
  double dist;
  double theta;
  double PropS;
  
  //for Rcpp random numbers
  RNGScope scope;
  
  Function sample("sample");
  //Function rcauchy("rcauchy");  
  
  //LOOP THROUGH EACH CELL of the input matrix 'spore_matrix' (this should be the study area)
  for (int row = 0; row < nrow; row++) {
    for (int col = 0; col < ncol; col++){
      
      if(spore_matrix(row,col) > 0){  //if spores in cell (row,col) > 0, disperse
        
        for(int sp = 1; (sp <= spore_matrix(row,col)); sp++){
          
          //GENERATE DISTANCES:
          if (rtype == "Cauchy") 
            dist = abs(R::rcauchy(0, scale1));
          else if (rtype == "Cauchy Mixture"){
            if (gamma >= 1 || gamma <= 0) stop("The parameter gamma must range between (0-1)");
            NumericVector fv = sample(Range(1, 2), 1, false, NumericVector::create(gamma, 1-gamma));
            int f = fv[0];
            if(f == 1) 
              dist = abs(R::rcauchy(0, scale1));
            else 
              dist = abs(R::rcauchy(0, scale2));
          }
          else stop("The parameter rtype must be set to either 'Cauchy' or 'Cauchy Mixture'");
          
          //GENERATE ANGLES (using Uniform distribution):
          theta = R::runif(-PI, PI);
          
          //calculate new row and col position for the dispersed spore unit (using dist and theta)
          row0 = row - round((dist * cos(theta)) / rs);
          col0 = col + round((dist * sin(theta)) / rs);
          
          if (row0 < 0 || row0 >= nrow) continue;     //outside the region
          if (col0 < 0 || col0 >= ncol) continue;     //outside the region
          
          //if distance is within same pixel challenge all SOD hosts, otherwise challenge UMCA only
          if (row0 == row && col0 == col){
            
            //if susceptible hosts are present in cell, calculate prob of infection
            if(S_host1_mat(row0, col0) > 0 || S_host2_mat(row0, col0) > 0 || S_LD(row0, col0) > 0 ){
              
              //WHAT IS THE PROBABILITY THAT A SPORE HITS ANY SUSCEPTIBLE HOST?
              int S_TOTAL = S_host1_mat(row0, col0) + S_host2_mat(row0, col0) + S_LD(row0, col0);
              
              PropS = double (S_TOTAL) / N_LVE(row0, col0);
              
              double U = R::runif(0,1);			  
              
              //if U < PropS then one of the susceptible trees will get hit
              if (U < PropS){
                
                //WHICH SUSCEPTIBLE TREE GETS HIT? CALCULATE PROBABILITY WEIGHTS
                double Prob_UM = double (S_host1_mat(row0, col0)) / S_TOTAL; 
                double Prob_LD = double (S_LD(row0, col0)) / S_TOTAL;
                double Prob_OK = double (S_host2_mat(row0, col0)) / S_TOTAL;
                
                //sample which of the three hosts will be hit given probability weights
                IntegerVector sv = sample(seq_len(3), 1, false, 
                                          NumericVector::create(Prob_UM, Prob_LD, Prob_OK));
                
                //WHAT IS THE PROBABILITY THAT A SPORE TURNS INTO AN INFECTION, GIVEN THAT A SUSCEPTIBLE TREE HAS BEEN HIT?
                //This depends on both the local weather AND the host competency score (relative to UMCA and OAKS)
                int s = sv[0];
                if (s == 1){
                  double ProbINF = Prob_UM * weather_suitability(row0, col0);
                  if (U < ProbINF){
                    I_host1_mat(row0, col0) = I_host1_mat(row0, col0) + 1; //update infected UMCA
                    S_host1_mat(row0, col0) = S_host1_mat(row0, col0) - 1; //update susceptible UMCA
                  }
                }else if (s == 2){
                  double ProbINF = Prob_LD * weather_suitability(row0, col0);
                  if (U < ProbINF){
                    I_LD(row0, col0) = I_LD(row0, col0) + 1; //update infected LIDE
                    S_LD(row0, col0) = S_LD(row0, col0) - 1; //update susceptible LIDE                    
                  }
                }else{
                  double ProbINF = Prob_OK * weather_suitability(row0, col0) * 0.75; //hardcoded coeff to decrease prob.infection (transmission)
                  if (U < ProbINF){
                    I_host2_mat(row0, col0) = I_host2_mat(row0, col0) + 1; //update infected OAKS
                    S_host2_mat(row0, col0) = S_host2_mat(row0, col0) - 1; //update susceptible OAKS  				
                  }
                }
              }//END IF CHECK vs UNIFORM NUMBER	                
              
            }//END IF NO SUSCEPTIBLE PRESENT IN CELL
            
          }else{  //IF DISTANCE IS OUTSIDE THE SAME CELL
            
            if(S_host1_mat(row0, col0) > 0 || S_LD(row0, col0) > 0){  //IF SUSCEPTIBLE HOST IS AVAILABLE (UMCA OR LIDE)
              
              //WHAT IS THE PROBABILITY THAT A SPORE HITS ANY SUSCEPTIBLE HOST?
              PropS = double(S_host1_mat(row0, col0) + S_LD(row0, col0)) / N_LVE(row0, col0);
              
              double U = R::runif(0,1);		
              
              //if U < ProbS then one of the susceptible trees will get hit
              if (U < PropS){
                
                //WHICH SUSCEPTIBLE TREE GETS HIT? CALCULATE PROBABILITY WEIGHTS
                double Prob_UM = double (S_host1_mat(row0, col0)) / (S_host1_mat(row0, col0) + S_LD(row0, col0)); 
                double Prob_LD = double (S_LD(row0, col0)) / (S_host1_mat(row0, col0) + S_LD(row0, col0));
                
                //sample which of the three hosts will be hit given probability weights
                IntegerVector sv = sample(seq_len(2), 1, false, 
                                          NumericVector::create(Prob_UM, Prob_LD));
                
                //WHAT IS THE PROBABILITY THAT A SPORE TURNS INTO AN INFECTION, GIVEN THAT A SUSCEPTIBLE TREE HAS BEEN HIT?
                //This depends on both the local weather AND the host competency score (relative to UMCA and OAKS)
                int s = sv[0];
                if (s == 1){
                  double ProbINF = Prob_UM * weather_suitability(row0, col0);
                  if (U < ProbINF){
                    I_host1_mat(row0, col0) = I_host1_mat(row0, col0) + 1; //update infected UMCA
                    S_host1_mat(row0, col0) = S_host1_mat(row0, col0) - 1; //update susceptible UMCA
                  }
                }else{
                  double ProbINF = Prob_LD * weather_suitability(row0, col0);
                  if (U < ProbINF){
                    I_LD(row0, col0) = I_LD(row0, col0) + 1; //update infected LIDE
                    S_LD(row0, col0) = S_LD(row0, col0) - 1; //update susceptible LIDE                    
                  }                
                }  
              }//END IF U < PROB
            }//ENF IF S > 0  
          } //END IF DISTANCE CHECK
          
        }//END LOOP OVER ALL SPORES IN CURRENT CELL GRID     
      }//END IF SPORES IN CURRENT CELL
    }   
  }//END LOOP OVER ALL GRID CELLS
  
  //return List::create(Named("I")=I, Named("S")=S);
  return List::create(
    _["S_host1_mat"] = S_host1_mat, 
    _["I_host1_mat"] = I_host1_mat,
    _["S_host2_mat"] = S_host2_mat, 
    _["I_host2_mat"] = I_host2_mat,
    _["S_LD"] = S_LD, 
    _["I_LD"] = I_LD
  );
  
} //END OF FUNCTION				  

// [[Rcpp::export]]
List SporeDispCppWind_MH(IntegerMatrix spore_matrix, 
                         IntegerMatrix S_host1_mat, IntegerMatrix S_LD, IntegerMatrix S_host2_mat, 
                         IntegerMatrix I_host1_mat, IntegerMatrix I_LD, IntegerMatrix I_host2_mat, 
                         IntegerMatrix N_LVE,
                         NumericMatrix weather_suitability,   //use different name than the functions in myfunctions_SOD.r
                         double rs, String rtype, double scale1, 
                         String wdir, int kappa,
                         double scale2=NA_REAL,  //default values
                         double gamma=NA_REAL)   //default values
{ 
  
  // internal variables //
  int nrow = spore_matrix.nrow(); 
  int ncol = spore_matrix.ncol();
  int row0;
  int col0;
  
  double dist;
  double theta;
  double PropS;
  
  //for Rcpp random numbers
  RNGScope scope;
  
  //Function rcauchy("rcauchy");
  Function rvm("rvm");
  Function sample("sample");
  
  //LOOP THROUGH EACH CELL of the input matrix 'spore_matrix' (this should be the study area)
  for (int row = 0; row < nrow; row++) {
    for (int col = 0; col < ncol; col++){
      
      if(spore_matrix(row,col) > 0){  //if spores in cell (row,col) > 0, disperse
        
        for(int sp = 1; (sp <= spore_matrix(row,col)); sp++){
          
          //GENERATE DISTANCES:
          if (rtype == "Cauchy") 
            dist = abs(R::rcauchy(0, scale1));
          else if (rtype == "Cauchy Mixture"){
            if (gamma >= 1 || gamma <= 0) stop("The parameter gamma must range between (0-1)");
            NumericVector fv = sample(Range(1, 2), 1, false, NumericVector::create(gamma, 1-gamma));
            int f = fv[0];
            if(f == 1) 
              dist = abs(R::rcauchy(0, scale1));
            else 
              dist = abs(R::rcauchy(0, scale2));
          }
          else stop("The parameter rtype must be set to either 'Cauchy' or 'Cauchy Mixture'");
          
          //GENERATE ANGLES (using Von Mises distribution):
          if(kappa <= 0)  // kappa=concentration
            stop("kappa must be greater than zero!");
          
          //predominant wind dir
          if (wdir == "N") 
            theta = as<double>(rvm(1, 0 * (PI/180), kappa));  
          else if (wdir == "NE")
            theta = as<double>(rvm(1, 45 * (PI/180), kappa));  
          else if(wdir == "E")
            theta = as<double>(rvm(1, 90 * (PI/180), kappa));  
          else if(wdir == "SE")
            theta = as<double>(rvm(1, 135 * (PI/180), kappa));  
          else if(wdir == "S")
            theta = as<double>(rvm(1, 180 * (PI/180), kappa));  
          else if(wdir == "SW")
            theta = as<double>(rvm(1, 225 * (PI/180), kappa));  
          else if(wdir == "W")
            theta = as<double>(rvm(1, 270 * (PI/180), kappa));  
          else
            theta = as<double>(rvm(1, 315 * (PI/180), kappa));
          
          //calculate new row and col position for the dispersed spore unit (using dist and theta)
          row0 = row - round((dist * cos(theta)) / rs);
          col0 = col + round((dist * sin(theta)) / rs);
          
          if (row0 < 0 || row0 >= nrow) continue;     //outside the region
          if (col0 < 0 || col0 >= ncol) continue;     //outside the region
          
          //if distance is within same pixel challenge all SOD hosts, otherwise challenge UMCA only
          if (row0 == row && col0 == col){
            
            //if susceptible hosts are present in cell, calculate prob of infection
            if(S_host1_mat(row0, col0) > 0 || S_host2_mat(row0, col0) > 0 || S_LD(row0, col0) > 0){
              
              //WHAT IS THE PROBABILITY THAT A SPORE HITS ANY SUSCEPTIBLE HOST?
              int S_TOTAL = S_host1_mat(row0, col0) + S_host2_mat(row0, col0) + S_LD(row0, col0);
              
              PropS = double (S_TOTAL) / N_LVE(row0, col0);
              
              double U = R::runif(0,1);			  
              
              //if U < PropS then one of the susceptible trees will get hit
              if (U < PropS){
                
                //WHICH SUSCEPTIBLE TREE GETS HIT? CALCULATE PROBABILITY WEIGHTS
                double Prob_UM = double (S_host1_mat(row0, col0)) / S_TOTAL; 
                double Prob_LD = double (S_LD(row0, col0)) / S_TOTAL;
                double Prob_OK = double (S_host2_mat(row0, col0)) / S_TOTAL;
                
                //sample which of the three hosts will be hit given probability weights
                IntegerVector sv = sample(seq_len(3), 1, false, 
                                          NumericVector::create(Prob_UM, Prob_LD, Prob_OK));
                
                int s = sv[0];
                if (s == 1){
                  double ProbINF = Prob_UM * weather_suitability(row0, col0);
                  if (U < ProbINF){
                    I_host1_mat(row0, col0) = I_host1_mat(row0, col0) + 1; //update infected UMCA
                    S_host1_mat(row0, col0) = S_host1_mat(row0, col0) - 1; //update susceptible UMCA
                  }
                }else if (s == 2){
                  double ProbINF = Prob_LD * weather_suitability(row0, col0);
                  if (U < ProbINF){
                    I_LD(row0, col0) = I_LD(row0, col0) + 1; //update infected LIDE
                    S_LD(row0, col0) = S_LD(row0, col0) - 1; //update susceptible LIDE                    
                  }
                }else{
                  double ProbINF = Prob_OK * weather_suitability(row0, col0) * 0.75; //hardcoded coeff to decrease prob.infection (transmission)
                  if (U < ProbINF){
                    I_host2_mat(row0, col0) = I_host2_mat(row0, col0) + 1; //update infected OAKS
                    S_host2_mat(row0, col0) = S_host2_mat(row0, col0) - 1; //update susceptible OAKS  				
                  }
                }
              }//END IF CHECK vs UNIFORM NUMBER	                
              
            }//END IF NO SUSCEPTIBLE PRESENT IN CELL
            
          }else{  //IF DISTANCE IS OUTSIDE THE SAME CELL
            
            if(S_host1_mat(row0, col0) > 0 || S_LD(row0, col0) > 0){  //IF SUSCEPTIBLE HOST IS AVAILABLE (UMCA OR LIDE)
              
              //WHAT IS THE PROBABILITY THAT A SPORE HITS ANY SUSCEPTIBLE HOST?
              PropS = double(S_host1_mat(row0, col0) + S_LD(row0, col0)) / N_LVE(row0, col0);
              
              double U = R::runif(0,1);		
              
              //if U < ProbS then one of the susceptible trees will get hit
              if (U < PropS){
                
                //WHICH SUSCEPTIBLE TREE GETS HIT? CALCULATE PROBABILITY WEIGHTS
                double Prob_UM = double (S_host1_mat(row0, col0)) / (S_host1_mat(row0, col0) + S_LD(row0, col0)); 
                double Prob_LD = double (S_LD(row0, col0)) / (S_host1_mat(row0, col0) + S_LD(row0, col0));
                
                //sample which of the three hosts will be hit given probability weights
                IntegerVector sv = sample(seq_len(2), 1, false, 
                                          NumericVector::create(Prob_UM, Prob_LD));
                
                //WHAT IS THE PROBABILITY THAT A SPORE TURNS INTO AN INFECTION, GIVEN THAT A SUSCEPTIBLE TREE HAS BEEN HIT?
                //This depends on both the local weather AND the host competency score (relative to UMCA and OAKS)
                int s = sv[0];
                if (s == 1){
                  double ProbINF = Prob_UM * weather_suitability(row0, col0);
                  if (U < ProbINF){
                    I_host1_mat(row0, col0) = I_host1_mat(row0, col0) + 1; //update infected UMCA
                    S_host1_mat(row0, col0) = S_host1_mat(row0, col0) - 1; //update susceptible UMCA
                  }
                }else{
                  double ProbINF = Prob_LD * weather_suitability(row0, col0);
                  if (U < ProbINF){
                    I_LD(row0, col0) = I_LD(row0, col0) + 1; //update infected LIDE
                    S_LD(row0, col0) = S_LD(row0, col0) - 1; //update susceptible LIDE                    
                  }                
                }  
              }//END IF U < PROB
            }//ENF IF S > 0  
          } //END IF DISTANCE CHECK
          
        }//END LOOP OVER ALL SPORES IN CURRENT CELL GRID     
      }//END IF SPORES IN CURRENT CELL
    }   
  }//END LOOP OVER ALL GRID CELLS
  
  //return List::create(Named("I")=I, Named("S")=S);
  return List::create(
    _["S_host1_mat"] = S_host1_mat, 
    _["I_host1_mat"] = I_host1_mat,
    _["S_host2_mat"] = S_host2_mat, 
    _["I_host2_mat"] = I_host2_mat,
    _["S_LD"] = S_LD, 
    _["I_LD"] = I_LD
  );
  
} //END OF FUNCTION		