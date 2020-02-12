#--------------------------------------------------------------------#
#       Project: Accelerometer Raw Data Analyzer and Smoother        #
#                    Author: Skyler Colledge                         #
#                 Open Source Code, 02/08/2020                       #
#--------------------------------------------------------------------#

if(!is.element('zeallot', installed.packages()[,1]))
{install.packages('zeallot', dependencies = TRUE)
} else {print("zeallot library already installed")}

extraction_dir = "home/pi/Documents"
saving_dir = ""

# Ask the user whether or not to use the default or other defined directory
directory_question <- readline(prompt = "Do you want to use the predefined directory? [Y or N]: ")

# If yes, break
if (directory_question == 'Y') {
  break
} else {
  extraction_dir <- readline(prompt = "Under which working directory do you want to extract data from? ")
}


setwd("/home/skyler/Downloads")
file <- file.path('Raw Data - Sheet1.csv')

# Take data out of .csv and put the columns of data into lists
accel.data <- read.csv(file, header = TRUE, sep = ',')

# Compute the total acceleration and write it to a different file
compute_total_acceleration <- function(accel.data) {
  total_accel <- sqrt((accel.data[,1])^2 + (accel.data[,2])^2 + (accel.data[,3])^2)
  return(total_accel)
}

compute_difference <- function(accel.data) {
  difference.x <- accel.data[,2] - accel.data[,5]
  difference.y <- accel.data[,3] - accel.data[,6]
  difference.z <- accel.data[,4] - accel.data[,7]
  difference.list <- list(difference.x, difference.y, difference.z)
  return(difference.list)
}

compute_total_difference <- function(data) {
  
}

tot.accel.sensor1 <- compute_total_acceleration(accel.data[,2:4])
tot.accel.sensor2 <- compute_total_acceleration(accel.data[,5:7])
tot.accel <- data.frame(accel.data[,1], tot.accel.sensor1, tot.accel.sensor2)

# Takes the average of the data to determine the percent acceleration absorbed
percent.reduction <- mean(tot.accel.sensor1) / mean(tot.accel.sensor2)
print(percent.reduction)

tot.difference <- compute_difference(accel.data)

plot()
write.table(tot.accel, "data.csv", sep = ',', dec = '.')
