#include "bil_model.h"

using namespace std;

//!mezní teploty pro jednotlivé vegetační zóny
const double bilan::T_veg_zone[bilan::veg_zones_count - 1] = {0, 5.3, 7.3, 9, 12.8};

/**
 * - potential evapotranspiration estimation by using latitude and temperature
 * @param latitude latitude in degrees
 */
void bilan::pet_estim_latit(long double latitude)
{
  unsigned begin_doy = 0, end_doy = 0;
  long double rad_lat, Gsc, dr, delta, om, Ra, t5, tmp_PET;
  date begin_month, end_month;

  if (!var)
    throw bil_err("Variables are not initialized for PET estimation.");
  if (!var_is_input[T])
    throw bil_err("Temperature needed for PET estimation is missing.");

  this->latitude = latitude;
  for (ts = 0; ts < time_steps; ts++) {
    rad_lat = M_PI / 180 * latitude;
    Gsc = 0.0820;

    switch (type) {
      case DAILY:
        begin_doy = calen[ts].get_day_of_year();
        end_doy = begin_doy;
        break;
      case MONTHLY: //sum of PETs for all days in month
        begin_month = calen[ts];
        begin_month.day = 1;
        end_month = calen[ts];
        end_month.increase(date::MONTH);
        end_month.day = 1;
        end_month.decrease(date::DAY);
        begin_doy = begin_month.get_day_of_year();
        end_doy = end_month.get_day_of_year();
        break;
      default:
        break;
    }

    unsigned days_in_year = 365;
    if (calen[ts].is_leap())
      days_in_year = 366;

    var[ts][PET] = 0;
    for (unsigned doy = begin_doy; doy <= end_doy; doy++) {
      dr = 1 + 0.033 * cos(doy * 2 * M_PI / days_in_year);
      delta = 0.409 * sin(doy * 2 * M_PI / days_in_year - 1.39);
      om = acos(-tan(rad_lat) * tan(delta));
      Ra = (24 * 60) / M_PI * Gsc * dr * (om * sin(rad_lat) * sin(delta) + cos(rad_lat) * cos(delta) * sin(om));

      t5 = var[ts][T] + 5;
      tmp_PET = 0.408 * Ra * t5 / 100;
      if (tmp_PET > 0)
        var[ts][PET] += tmp_PET;
    }
  }
  var_is_input[PET] = true;
}

/**
 * - potential evapotranspiration estimation by using tables for vegetation zones
 */
void bilan::pet_estim_tab()
{
  int veg_zone; /*typ vegetacni zony*/
  double Tsum = 0, Tmean;

  if (!var)
    throw bil_err("Variables are not initialized for PET estimation.");
  if (!var_is_input[T] || !var_is_input[H])
    throw bil_err("Temperature or humidity needed for PET estimation is missing.");

  //zjištění průměrné teploty
  for (ts = 0; ts < time_steps; ts++)
    Tsum += var[ts][T];
  Tmean = Tsum / time_steps;

  //veg. zóna pro horní mez - u posledního a prvního obě meze stejné
  veg_zone = TUNDRA; //if Tmean < T_veg_zone[TUNDRA]
  for (int vz = TUNDRA; vz < STEP; vz++) {
    if (Tmean > T_veg_zone[vz])
      veg_zone = vz + 1;
  }

  /*funkce pro vypocet vyparu pro dve veg. zony*/
  //upper limit the same excepting the last zone
  int tmp_veg_zone = veg_zone;
  if (veg_zone == STEP)
    tmp_veg_zone = veg_zone - 1;
  pet_estim_tab_zone(tmp_veg_zone);

  //store results for the first zone
  long double* tmp_PET = new long double[time_steps];
  for (ts = 0; ts < time_steps; ts++)
    tmp_PET[ts] = var[ts][PET];

  /*kdyz je jen 1 zona, nepocita se dal*/
  if (veg_zone != TUNDRA && veg_zone != STEP) {
    //lower limit
    tmp_veg_zone = veg_zone - 1;
    pet_estim_tab_zone(tmp_veg_zone);
  }

  switch (veg_zone) {
    case TUNDRA:
    case STEP:
      if (type == DAILY) {
        for (ts = 0; ts < time_steps; ts++)
          var[ts][PET] = var[ts][PET] / 30; /*na jednotlivy dny, tabulky jsou pro mesice*/
      }
      break;
    case JEHL:
    case SMIS:
    case LIST:
    case LESOSTEP:
      /*linearni interpolace mezi vegetacnimi zonami*/
      for (ts = 0; ts < time_steps; ts++) {
        var[ts][PET] = var[ts][PET] + (Tmean - T_veg_zone[veg_zone - 1]) * (tmp_PET[ts] - var[ts][PET]) / (T_veg_zone[veg_zone] - T_veg_zone[veg_zone - 1]);
        if (type == DAILY)
          var[ts][PET] = var[ts][PET] / 30;
      }
      break;
    default:
      break;
  }
  delete[] tmp_PET;
  var_is_input[PET] = true;
}

/**
 * - potential evapotranspiration from tables for one vegetation zone
 * @param veg_zone type of vegetation zone
 */
void bilan::pet_estim_tab_zone(int veg_zone)
{
  //PET dependancy on saturation deficit
  //columns: months from January to December
  //rows: first number is number of items (= max. saturation deficit), then PET values for sd 0,1...maximum
  long double tundra[][12] =
  {
    {8,8,8,8,8,8,8,8,8,8,8,8},
    {2,4.5,7.5,23,23,11,6,3,1,1,1,1},
    {9,15.5,30,60,60,40,23,13.5,5,3,3,5},
    {15,25.5,47.5,78,78,57,37,22,9,5.5,5.5,9},
    {21,35,58,90,90,69,50,30,12,7.5,7.5,12},
    {26,44,67.5,99,99,77.5,59,37,14.5,9,9,14.5},
    {31.5,51,75,107.5,107.5,85.5,65.5,44,17,10,10,17},
    {36,57.5,82.5,114.5,114.5,94,71,50,19,11,11,19},
    {40,64,90,120,120,100,75,55.5,21.5,12,12,21.5}
  };
  long double jehlicnate[][12] =
  {
    {11,11,11,11,11,11,11,11,11,11,11,11},
    {2,3,4,5,10,20,20,13,8,5,3,2},
    {5,8,12,22,33,55,55,37,26,18,8,5},
    {8,13,19,36,51,74,74,56,42,29,13,8},
    {11,18,26,48,67,87,87,69,55,39,18,11},
    {14,24,32,58,81,98,98,79,66,48,24,14},
    {17,28,36,67,94,107,107,88,74,55,28,17},
    {20,32,40,73,104,115,115,95,81,60,32,20},
    {22,36,45,78,112,123,123,103,86,65,36,22},
    {25,39,48,82,119,129,129,108,92,69,39,25},
    {27,43,51,86,125,135,135,114,96,73,43,27},
    {35,46,55,90,132,142,142,119,101,76,46,35},
  };
  long double smisene[][12] =
  {
    {11,11,11,11,11,11,11,11,11,11,11,11},
    {3,3,4,10,17,28,22,12,7,5,3,3},
    {8,10,14,32,46,65,58,39,26,18,10,8},
    {14,16,23,50,67,84,78,58,43,31,16,14},
    {19,23,30,64,82,95,91,72,55,42,23,19},
    {25,30,37,75,93,105,101,84,65,52,30,25},
    {31,36,45,85,103,114,110,93,75,61,36,31},
    {37,42,52,94,111,122,117,102,83,69,42,37},
    {41,48,58,101,118,129,125,110,89,75,48,41},
    {47,53,65,108,125,136,132,117,96,82,53,47},
    {52,59,72,114,133,142,138,124,102,88,59,52},
    {55,64,77,120,139,148,144,129,108,94,64,55}
  };
  long double listnate[][12] =
  {
    {11,11,11,11,11,11,11,11,11,11,11,11},
    {4,5,6,10,20,33,26,15,10,6,5,4},
    {11,14,20,35,53,71,63,43,29,20,14,11},
    {18,23,33,53,73,88,81,62,46,33,23,18},
    {29,31,45,67,86,98,92,75,59,45,31,29},
    {30,38,55,78,96,107,101,86,70,55,38,30},
    {36,47,63,87,104,116,110,96,80,63,47,36},
    {42,55,72,96,112,124,117,104,89,72,55,42},
    {48,63,79,104,119,130,124,112,97,79,63,48},
    {53,68,87,111,126,136,131,118,104,87,68,53},
    {58,74,93,118,133,144,138,126,110,93,74,58},
    {62,78,98,124,139,150,144,133,126,98,78,62}
  };
  long double lesostep[][12] =
  {
    {19,19,19,19,19,19,19,19,19,19,19,19},
    {3,4,5,6,18,35,26,13,5,5,4,3},
    {13,17,23,35,54,72,66,43,29,20,17,13},
    {21,29,40,55,73,87,81,64,46,35,29,21},
    {30,40,54,68,85,98,93,77,60,46,40,30},
    {37,49,65,80,95,108,104,88,71,57,49,37},
    {45,57,75,90,105,116,112,97,83,66,57,45},
    {50,65,83,99,112,124,119,105,92,75,65,50},
    {57,71,90,106,120,131,126,113,100,83,71,57},
    {63,77,97,114,126,138,134,120,107,90,77,63},
    {68,83,105,121,134,145,140,127,115,96,83,68},
    {74,88,111,126,139,150,145,133,120,102,88,74},
    {80,94,116,133,145,155,151,139,126,107,94,80},
    {84,98,121,138,150,160,156,144,132,113,98,84},
    {89,103,126,143,155,165,161,149,137,118,103,89},
    {94,108,131,148,160,170,166,154,142,123,108,94},
    {99,113,136,153,168,175,170,159,147,127,113,99},
    {104,118,140,157,158,179,174,163,151,132,118,104},
    {108,202,145,161,173,183,178,166,155,141,202,108},
    {112,206,149,165,177,185,182,171,158,145,206,112}
  };

  long double (*tabulka)[12];
  long double t, pom, pom2, exp1, exp2, exp3; /*nejake pomocne*/
  long double et, dop, vypv, rozdil;

  switch (veg_zone) {
    case TUNDRA:
      tabulka = tundra;
      break;
    case JEHL:
      tabulka = jehlicnate;
      break;
    case SMIS:
      tabulka = smisene;
      break;
    case LIST:
      tabulka = listnate;
      break;
    case LESOSTEP:
      tabulka = lesostep;
      break;
    default:
      throw bil_err("Unknown vegetation zone.");
      break;
  }

  /*Vypocet vyparu z teploty a vlhkosti*/
  /*teplota,vlhkost->sytostni doplnek*/
  for (ts = 0; ts < time_steps; ts++) {
    /*vypocet maximálního tlaku vodní páry dle Coufala*/
    t = var[ts][T] + 273.16;
    pom = 273.16 / t;
    pom2 = 1 / pom;
    if (var[ts][T] > 0) {
      exp1 = 10.79574 * (1 - pom) - 0.4342945 * 5.028 * log(pom2);
      exp2 = 1.50475 * 0.0001 * (1 - pow(10, (-8.22969 * (pom2 - 1))));
      exp3 = 0.42873 * 0.001 * (pow(10, (4.76955 * (1 - pom))) - 1) + 0.78614;
    }
    else {
      exp1 = -9.09685 * (pom - 1);
      exp2 = -3.56654 * 0.4342945 * log(pom);
      exp3 = 0.87682 * (1 - pom2) + 0.78614;
    }

    et = pow(10, (exp1 + exp2 + exp3)); //maximální tlak vodní páry
    dop = et * (100 - var[ts][H]) / 100;  //sytostní doplněk

    if (dop < 0) {
      throw bil_err("Physically impossible: negative value of saturation deficit.\n");
    }
    if (dop > tabulka[0][calen[ts].month - 1] - 1) { //bere maximum, které je ale o 1 menší než počet, protože první číslo je pro SD=0
      dop = tabulka[0][calen[ts].month - 1] - 1; /*posledni, maximalni doplnek v radku je o 1 mensi nez pocet, protoze prvni SD je 0, ale dop se stejne neuklada*/
      vypv = tabulka[((int) tabulka[0][calen[ts].month - 1])][calen[ts].month - 1]; /*vypocteny vypar maximalni v tabulce*/
    }
    else {
      /*interpolace vyparu z okolnich hodnot sytostniho doplnku*/
      int sd_horni; //hodnota sytostního doplňku - horní mez, ze které se interpoluje
      //zároveň odpovídá indexu pole-tabulky pro dolní mez
      sd_horni = 1;
      while (dop >= sd_horni) {
        sd_horni++;
      }
      rozdil = tabulka[sd_horni + 1][calen[ts].month - 1] - tabulka[sd_horni][calen[ts].month - 1];
      vypv = tabulka[sd_horni][calen[ts].month - 1] + rozdil * (dop - ((long double) sd_horni) + 1);
    }
    var[ts][PET] = vypv;
  } //ts
}
