#include <torch/torch.h>
#include <xtorch/xtorch.h>
#include <iostream>
#include <vector>
#include <functional>
#include <chrono>

using namespace std;

/**
 * @brief Main function to train a LeNet-5 model on the MNIST dataset using PyTorch.
 *
 * This program loads the MNIST dataset, applies transformations (resize and normalization),
 * creates a data loader with shuffling, initializes a LeNet-5 model, sets up an Adam optimizer,
 * and trains the model using a custom Trainer class with negative log likelihood loss.
 *
 * @return int Returns 0 on successful execution.
 */
int main()
{
    // Set precision for floating-point output
    std::cout.precision(10);

    /**
     * Load and transform the MNIST training dataset.
     * - Path: /home/kami/Documents/temp/
     * - Mode: Training
     * - Download: Enabled
     * - Transformations: Resize to 32x32, Normalize with mean 0.5 and std 0.5
     */
    auto dataset = xt::data::datasets::CIFAR10(
        "/home/kami/Documents/datasets/", DataMode::TRAIN, true,
        {
            xt::data::transforms::Resize({227, 227}),
            torch::data::transforms::Normalize<>(0.5, 0.5)
        }).map(torch::data::transforms::Stack<>());

    /**
     * Create a DataLoader for batching and shuffling the dataset.
     * - Batch size: 64
     * - Drop last incomplete batch: False
     * - Shuffle: Enabled
     */
    xt::DataLoader<decltype(dataset)> loader(
        std::move(dataset),
        torch::data::DataLoaderOptions().batch_size(64).drop_last(false),
        true);

    /**
     * Initialize the LeNet-5 model for 10 classes and move it to CPU.
     * Set the model to training mode.
     */
    const torch::Device device = torch::Device(torch::kCUDA);
    xt::models::AlexNet model(10, 3);

    model.to(device);
    model.train();

    /**
     * Set up the Adam optimizer with a learning rate of 1e-3.
     */
    torch::optim::Adam optimizer(model.parameters(), torch::optim::AdamOptions(1e-3));

    /**
     * Configure the Trainer with:
     * - Optimizer: Adam
     * - Maximum epochs: 5
     * - Loss function: Negative log likelihood loss
     */
    xt::Trainer trainer;
    trainer.set_optimizer(&optimizer)
           .set_max_epochs(10)
           .set_loss_fn([](auto output, auto target)
           {
               return torch::nll_loss(output, target);
           });

    /**
     * Train the model using the Trainer and DataLoader.
     */
    auto start_time = std::chrono::high_resolution_clock::now();

    trainer.fit<decltype(dataset)>(&model, loader , device);
    auto end_time = std::chrono::high_resolution_clock::now();

    // Calculate the duration in microseconds
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);

    // Output the duration in microseconds
    std::cout << "Program execution time: " << ((double)duration.count())/1000000 << " seconds" << std::endl;
    return 0;
}
