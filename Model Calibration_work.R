## Calibrate and Validate model results

## Set up and test Spotted lattern fly model
pest_vars <<- list(host1_rast = NULL,host1_score = NULL, host2_rast=NULL,host2_score=NULL,host3_rast=NULL,host3_score=NULL, host4_rast=NULL,host4_score=NULL,host5_rast=NULL,host5_score=NULL,
                   host6_rast=NULL,host6_score=NULL,host7_rast=NULL,host7_score=NULL,host8_rast=NULL,host8_score=NULL,host9_rast=NULL,host9_score=NULL,host10_rast=NULL,host10_score=NULL,
                   allTrees=NULL,initialPopulation=NULL, start=2000, end=2010, seasonality = 'NO', s1 = 1 , s2 = 12, sporeRate = 4.4, windQ =NULL, windDir=NULL, tempQ="NO", tempData=NULL, 
                   precipQ="NO", precipData=NULL, kernelType ='Cauchy', kappa = 2, number_of_hosts = 1, scale1 = 20.57, scale2 = NULL, gamma = 1, seed_n = 62, time_step = "weeks")
pest_vars$host1_rast = raster("C:/Users/cmjone25/Dropbox/Projects/APHIS/Ailanthus/ToF.tif")
pest_vars$allTrees = raster("C:/Users/cmjone25/Dropbox/Projects/APHIS/Ailanthus/totalhost.tif")
pest_vars$initialPopulation = raster ("C:/Users/cmjone25/Dropbox/Projects/APHIS/Ailanthus/2014Initial.tif")
pest_vars$start = 2015
pest_vars$end = 2017
pest_vars$seasonality = 'YES'
pest_vars$s1 = 6
pest_vars$s2 = 11
pest_vars$sporeRate =3.0 ## spore rate default of original attempt
pest_vars$tempData = 'C:/Users/cmjone25/Desktop/WeatherCoeff/c_coef_2015_2017_slfarea.tif'
pest_vars$host1_score = 10
pest_vars$number_of_hosts = 1
pest_vars$tempQ = "YES"
pest_vars$precipQ = "NO"
pest_vars$windQ = "NO"
pest_vars$kernelType = "Cauchy"
pest_vars$scale1 = 59
pest_vars$time_step = "months"
data2 <- do.call(pest, pest_vars)
scale = 59
sporeRate = 3.0
seed_n = 62
i=0
params2 <- data.frame(scale, sporeRate, seed_n, i)
scales <- 59
spores <- 3.0
seeds <- c(62, 85, 98,25,34,150,155,89,67,12,13,99,47,43,52,74,20,38,91,121)
for (scale in scales) {
  for (sporeRate in spores) {
    for (seed in seeds) {
      i = i + 1
      pest_vars <<- list(host1_rast = NULL,host1_score = NULL, host2_rast=NULL,host2_score=NULL,host3_rast=NULL,host3_score=NULL, host4_rast=NULL,host4_score=NULL,host5_rast=NULL,host5_score=NULL,
                         host6_rast=NULL,host6_score=NULL,host7_rast=NULL,host7_score=NULL,host8_rast=NULL,host8_score=NULL,host9_rast=NULL,host9_score=NULL,host10_rast=NULL,host10_score=NULL,
                         allTrees=NULL,initialPopulation=NULL, start=2000, end=2010, seasonality = 'NO', s1 = 1 , s2 = 12, sporeRate = 4.4, windQ =NULL, windDir=NULL, tempQ="NO", tempData=NULL, 
                         precipQ="NO", precipData=NULL, kernelType ='Cauchy', kappa = 2, number_of_hosts = 1, scale1 = 20.57, scale2 = NULL, gamma = 1, seed_n = 42, time_step = "weeks")
      pest_vars$host1_rast = raster("C:/Users/cmjone25/Dropbox/Projects/APHIS/Ailanthus/ToF.tif")
      pest_vars$allTrees = raster("C:/Users/cmjone25/Dropbox/Projects/APHIS/Ailanthus/totalhost.tif")
      pest_vars$initialPopulation = raster ("C:/Users/cmjone25/Dropbox/Projects/APHIS/Ailanthus/2014Initial.tif")
      pest_vars$start = 2015
      pest_vars$end = 2017
      pest_vars$seasonality = 'YES'
      pest_vars$s1 = 6
      pest_vars$s2 = 11
      pest_vars$sporeRate = sporeRate ## spore rate default of original attempt
      pest_vars$tempData = 'C:/Users/cmjone25/Desktop/WeatherCoeff/c_coef_2015_2017_slfarea.tif'
      pest_vars$host1_score = 10
      pest_vars$number_of_hosts = 1
      pest_vars$tempQ = "YES"
      pest_vars$precipQ = "NO"
      pest_vars$windQ = "NO"
      pest_vars$kernelType = "Cauchy"
      pest_vars$time_step = "months"
      pest_vars$scale1 = scale
      pest_vars$seed_n = seed
      data2[[i]] <- do.call(pest, pest_vars)
      params2[i,1] <- scale
      params2[i,2] <- sporeRate
      params2[i,3] <- seed
      params2[i,4] <- i
      print(i)
    }}}

slf2015 = readOGR("C:/Users/cmjone25/Dropbox/Projects/APHIS/Ailanthus/2015SLF_p.shp")
slf2016 = readOGR("C:/Users/cmjone25/Dropbox/Projects/APHIS/Ailanthus/2016SLF_p.shp")
slf2017 = readOGR("C:/Users/cmjone25/Dropbox/Projects/APHIS/Ailanthus/2017SLF_p.shp")
r <- pest_vars$host1_rast
params2$acc2015 <-0
params2$area2015 <-0
params2$acc2016 <-0
params2$area2016 <- 0
params2$acc2017 <-0
params2$area2017 <- 0

for (i in 1:length(data2)) {
    p15 <- data2[[i]][[2]][[1]]
    slf2015$model <- extract(p15, slf2015)
    slf2015$model[is.na(slf2015$model)] <-0
    params2$acc2015[i] <- sum(slf2015$model>0)/length(slf2015)
    p15 <- as.matrix(p15)
    p15[is.na(p15)] <- 0
    params2$area2015[i] <- sum(p15>0)
    p16 <- data2[[i]][[2]][[2]]
    slf2016$model <- extract(p16, slf2016)
    slf2016$model[is.na(slf2016$model)] <-0
    params2$acc2016[i] <- sum(slf2016$model>0)/length(slf2016)
    p16 <- as.matrix(p16)
    p16[is.na(p16)] <- 0
    params2$area2016[i] <- sum(p16>0)
    p17 <- data2[[i]][[2]][[3]]
    slf2017$model <- extract(p17, slf2017)
    slf2017$model[is.na(slf2017$model)] <-0
    params2$acc2017[i] <- sum(slf2017$model>0)/length(slf2017)
    p17 <- as.matrix(p17)
    p17[is.na(p17)] <- 0
    params2$area2017[i] <- sum(p17>40)
  print(i)
}

checks2 <- aggregate(params2, by = list(params2$scale, params2$sporeRate), mean)
checks2$totalacc <- (checks2$acc2015*length(slf2015)+checks2$acc2016*length(slf2016)+checks2$acc2017*length(slf2017))/(length(slf2015)+length(slf2016)+length(slf2017))
write.csv(checks, "C:/Users/Chris/Desktop/slftests.csv")
writeRaster(data[[150]][[2]], "C:/Users/Chris/Desktop/slftest2.tif", overwrite = TRUE, format = 'GTiff')
writeRaster(data3[[2]][[2]], "C:/Users/cmjone25/Desktop/slftest5.tif", overwrite = TRUE, format = 'GTiff')

## Set up and test SOD
pest_vars <<- list(host1_rast = NULL,host1_score = NULL, host2_rast=NULL,host2_score=NULL,host3_rast=NULL,host3_score=NULL, host4_rast=NULL,host4_score=NULL,host5_rast=NULL,host5_score=NULL,
                   host6_rast=NULL,host6_score=NULL,host7_rast=NULL,host7_score=NULL,host8_rast=NULL,host8_score=NULL,host9_rast=NULL,host9_score=NULL,host10_rast=NULL,host10_score=NULL,
                   allTrees=NULL,initialPopulation=NULL, start=2000, end=2010, seasonality = 'NO', s1 = 1 , s2 = 12, sporeRate = 4.4, windQ =NULL, windDir=NULL, tempQ="NO", tempData=NULL, 
                   precipQ="NO", precipData=NULL, kernelType ='Cauchy', kappa = 2, number_of_hosts = 1,scale1 = 20.57, scale2 = NULL, gamma = 1, seed_n = 42, time_step = "weeks")
pest_vars$host1_rast = raster("C:/Users/Chris/Desktop/California/UMCA_1000m.tif")
pest_vars$host2_rast = raster("C:/Users/Chris/Desktop/California/oaks_1000m.tif")
pest_vars$allTrees = raster("C:/Users/Chris/Desktop/California/Total_dens_1000m.tif")
pest_vars$initialPopulation = raster ("C:/Users/Chris/Desktop/California/InitialInfections.tif")
pest_vars$start = 1990
pest_vars$end = 2017
pest_vars$seasonality = 'YES'
pest_vars$s1 = 1
pest_vars$s2 = 9
pest_vars$sporeRate =4.4 ## spore rate default of original attempt
pest_vars$tempData = 'G:/DaymetUS/California/WeatherCoeff/c_coef_1990_2017_california.tif'
pest_vars$precipData = 'G:/DaymetUS/California/WeatherCoeff/m_coef_1990_2017_california.tif'
pest_vars$host1_score = 10
pest_vars$host2_score = 0
pest_vars$number_of_hosts = 2
pest_vars$tempQ = "YES"
pest_vars$precipQ = "YES"
pest_vars$windQ = "NO"
pest_vars$kernelType = "Cauchy"
pest_vars$scale1 = 20.57
pest_vars$scale2 = 9504
pest_vars$gamma = .95
pest_vars$time_step = "weeks"
data <- do.call(pest, pest_vars)
scale = 2
sporeRate = 2
seed_n = 22
i=0
params <- data.frame(scale, sporeRate, seed_n, i)
scales <- 59
spores <- 3.0
seeds <- c(42, 45)
for (scale in scales) {
  for (sporeRate in spores) {
    for (seed in seeds) {
      i = i + 1
      pest_vars <<- list(host1_rast = NULL,host1_score = NULL, host2_rast=NULL,host2_score=NULL,host3_rast=NULL,host3_score=NULL, host4_rast=NULL,host4_score=NULL,host5_rast=NULL,host5_score=NULL,
                         host6_rast=NULL,host6_score=NULL,host7_rast=NULL,host7_score=NULL,host8_rast=NULL,host8_score=NULL,host9_rast=NULL,host9_score=NULL,host10_rast=NULL,host10_score=NULL,
                         allTrees=NULL,initialPopulation=NULL, start=2000, end=2010, seasonality = 'NO', s1 = 1 , s2 = 12, sporeRate = 4.4, windQ =NULL, windDir=NULL, tempQ="NO", tempData=NULL, 
                         precipQ="NO", precipData=NULL, kernelType ='Cauchy', kappa = 2, number_of_hosts = 1, seed_n =42)
      pest_vars$host1_rast = raster("C:/Users/Chris/Dropbox/Projects/APHIS/Ailanthus/ToF.tif")
      pest_vars$allTrees = raster("C:/Users/Chris/Dropbox/Projects/APHIS/Ailanthus/totalhost.tif")
      pest_vars$initialPopulation = raster ("C:/Users/Chris/Dropbox/Projects/APHIS/Ailanthus/2014Initial.tif")
      pest_vars$start = 2015
      pest_vars$end = 2017
      pest_vars$seasonality = 'YES'
      pest_vars$s1 = 6
      pest_vars$s2 = 11
      pest_vars$sporeRate =sporeRate ## spore rate default of original attempt
      pest_vars$tempData = 'G:/DaymetUS/SLF_area/WeatherCoeff/c_coef_2015_2017_slfarea.tif'
      pest_vars$host1_score = 10
      pest_vars$number_of_hosts = 1
      pest_vars$tempQ = "YES"
      pest_vars$precipQ = "NO"
      pest_vars$windQ = "NO"
      pest_vars$kernelType = "Cauchy"
      pest_vars$scale1 = scale
      pest_vars$seed_n = seed
      data[[i]] <- do.call(pest, pest_vars)
      params[i,1] <- scale
      params[i,2] <- sporeRate
      params[i,3] <- seed
      params[i,4] <- i
      print(i)
    }}}




## Test changes to model for Generalizability
host1_rast = raster("C:/Users/Chris/Dropbox/Projects/APHIS/Ailanthus/ToF.tif")
allTrees = raster("C:/Users/Chris/Dropbox/Projects/APHIS/Ailanthus/totalhost.tif")
initialPopulation = raster ("C:/Users/Chris/Dropbox/Projects/APHIS/Ailanthus/2014Initial.tif")
start = 2015
end = 2017
time_step ="weeks"
seasonality = 'YES'
s1 = 6
s2 = 11
sporeRate =3 ## spore rate default of original attempt
tempData = 'G:/DaymetUS/SLF_area/WeatherCoeff/c_coef_2015_2017_slfarea.tif'
host1_score = 10
number_of_hosts = 1
tempQ = "YES"
precipQ = "NO"
windQ = "NO"
kernelType = "Cauchy"
scale1 = 50
seed_n = 20
time_step = "months"

host_score <- rep(host1_score,10)
