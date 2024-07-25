pipeline {
    agent any
    stages {
        stage('Build') {
            steps {
                // Build Maven project
                sh 'mvn clean package'
            }
        }
        stage('Test') {
            steps {
                // Run unit tests
                sh 'mvn test'
            }
        }
        stage('Archive') {
            steps {
                // Archive the built artifacts
                archiveArtifacts artifacts: '**/target/*.jar', allowEmptyArchive: true
            }
        }
    }
    
    post {
        success {
            // Notification or further actions on success
            echo 'Pipeline succeeded!'
        }
        failure {
            // Notification or further actions on failure
            echo 'Pipeline failed!'
        }
    }
}

